/*
 * Kernel module for HC-SR04 Ultrasonic Range Finder
 *
 * Original written by
 * Copyright (C) 2014 James Ward
 *
 * Port to support single signal pin device, iio, and device tree configuration
 *
 * Copyright (C) 2019 Seeed Studio
 * Peter Yang <turmary@126.com>
 *
 * Released under the GPL
 *
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/iio/iio.h>

#define DEV_NAME	"hcsr04"
#define DEV_ALERT	KERN_ALERT DEV_NAME ": "

struct hcsr04 {
	struct device *   dev;
	int               trig;                /* trigger pin */
	int               echo;                /* echo pin */
	int               count;               /* Number of interrupts received */
	ktime_t           time_stamp[2];       /* Time-stamp of interrupts */
	/* Declare a wait queue to wait for interrupts */
	wait_queue_head_t wait;
	/* Mutex used to prevent simultaneous access */
	struct mutex      lock;
	long              distance;
};


/*---------------------------------------------------------------------------*/

/* GPIO pin used for trigger out */
static int trig_pin = 57;
module_param(trig_pin, int, S_IRUGO);

/* GPIO pin used for echo in */
static int echo_pin = 57;
module_param(echo_pin, int, S_IRUGO);

/* High level pulse width, in us */
static int pulse = 20;
module_param(pulse, int, S_IRUGO);

/* Timeout for range finding operation in milliseconds */
const long TIMEOUT_RANGE_FINDING = 60;

/*---------------------------------------------------------------------------*/

/* Interrupt handler: called on rising/falling edge */
static irqreturn_t gpio_interrupt_handler( int irq, void *dev_id )
{
	struct hcsr04* hc = (struct hcsr04*)dev_id;

	/* Get the kernel time */
	ktime_t time_stamp = ktime_get();


	/* For the first two interrupts received, store the time-stamp */
	if ( hc->count < 2 )
		hc->time_stamp[hc->count] = time_stamp;

	/* Count the number of interrupts received */
	++hc->count;

	/* If we have received two interrupts, wake up */
	if ( hc->count > 1 )
		wake_up_interruptible( &hc->wait );

	return IRQ_HANDLED;
}

/*---------------------------------------------------------------------------*/

/* Measures the range, returning the time period in microseconds, and the
 * range in millimetres. The return value is 1 for success, and 0 for failure.
 */
static int hcsr04_measure_range(
	struct hcsr04* hc,
	long *us,	/* out: time in us */
	long *mm	/* out: range in mm */
) {
	/* The speed of sound in mm/s */
	const long speed_sound_mms = 340270;

	ktime_t elapsed_kt;          /* used to store elapsed time */
	struct timeval elapsed_tv;   /* elapsed time as timeval */
	int irq = 0;                 /* the IRQ number */
	int range_complete = 0;      /* indicates successful range finding */

	/* Acquire the mutex before entering critical section */
	mutex_lock( &hc->lock );

	/* Initialise variables used by interrupt handler */
	hc->count = 0;
	memset( &hc->time_stamp, 0, sizeof(hc->time_stamp) );

	if (hc->echo == hc->trig) {
		/* make GPIO an output */
		if ( gpio_direction_output(hc->trig, 0) != 0 ) {
			printk( DEV_ALERT "Failed to make sig/trig pin %d an output\n", hc->trig );
			mutex_unlock( &hc->lock );
			return -1;
		}
	}

	/* Transmit trigger pulse, lasting at least 20us */
	gpio_set_value(hc->trig, 0);
	udelay(2);
	gpio_set_value(hc->trig, 1);
	udelay(pulse);
	gpio_set_value(hc->trig, 0);

	if (hc->echo == hc->trig) {
		/* make GPIO an input */
		if ( gpio_direction_input(hc->echo) ) {
			printk( DEV_ALERT "Failed to make sig/echo pin %d an input\n", hc->echo );
			mutex_unlock( &hc->lock );
			return -1;
		}
	}

	/* Request an IRQ for the echo GPIO pin, so that we can measure the rising
	 * and falling edge of the pulse from the ranger.
	 */
	irq = gpio_to_irq( hc->echo );
	if ( request_irq(
		irq,
		gpio_interrupt_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_SHARED,
		"hcsr04_irq",
		hc
	) ) {
		/* Release the mutex */
		mutex_unlock( &hc->lock );
		return -1;
	}

	/* Wait until we have received two interrupts (indicating that
	 * range finding has completed), or we have timed out
	 */
	wait_event_interruptible_timeout(
		hc->wait,
		hc->count == 2,
		msecs_to_jiffies(TIMEOUT_RANGE_FINDING)
	);

	/* Free the interrupt */
	free_irq( irq, hc );

	/* Have we successfully completed ranging? */
	range_complete = (hc->count == 2);

	if ( range_complete ) {
		/* Calculate pulse length */
		elapsed_kt = ktime_sub( hc->time_stamp[1], hc->time_stamp[0] );
		elapsed_tv = ktime_to_timeval( elapsed_kt );
	}

	/* Release the mutex */
	mutex_unlock( &hc->lock );

	if ( range_complete ) {
		/* Return the time period in microseconds. We ignore the tv_sec,
		 * because the maximum delay should be less than 60ms
		 */
		if ( us != NULL )
			*us = elapsed_tv.tv_usec;

		/* Return the distance, based on speed of sound and the elapsed
		 * time in microseconds.
		 */
		if ( mm != NULL )
			*mm = elapsed_tv.tv_usec * (speed_sound_mms / 10) / 200000/*0*/;

		/* Success */
		return 1;
	} else {
		/* Failure */
		if ( us != NULL ) *us = 0;
		if ( mm != NULL ) *mm = 0;
		return 0;
	}
}

/*---------------------------------------------------------------------------*/

static int hcsr04_read_raw(struct iio_dev *iio,
        const struct iio_chan_spec *chan,
        int *val, int *val2, long m)
{
	struct hcsr04 *hcsr04 = iio_priv(iio);
	long us = 0;
	int ret;

	hcsr04_measure_range(hcsr04, &us, &hcsr04->distance);

	ret = IIO_VAL_INT;
	if (chan->type == IIO_DISTANCE)
		*val = hcsr04->distance;
	else
		ret = -EINVAL;

	return ret;
}

static const struct iio_info hcsr04_iio_info = {
	.driver_module		= THIS_MODULE,
	.read_raw		= hcsr04_read_raw,
};

static const struct iio_chan_spec hcsr04_chan_spec[] = {
	{ .type = IIO_DISTANCE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED), },
};

/*---------------------------------------------------------------------------*/

/* Initialise GPIO */
static int hcsr04_gpio_init( struct hcsr04* hc )
{
	/* check that trigger GPIO is valid */
	if ( !gpio_is_valid(hc->trig) ){
		printk( DEV_ALERT "trig GPIO %d is not valid\n", hc->trig );
		return -EINVAL;
	}

	/* request the GPIO pin */
	if ( gpio_request(hc->trig, DEV_NAME) != 0 ) {
		printk( DEV_ALERT "Unable to request trig GPIO %d\n", hc->trig );
		return -EINVAL;
	}

	/* make GPIO an output */
	if ( gpio_direction_output(hc->trig, 0) != 0 ) {
		printk( DEV_ALERT "Failed to make trig GPIO %d an output\n", hc->trig );
		return -EINVAL;
	}

	/* check if trigger and echo are the same GPIO */
	if ( hc->trig == hc->echo ) {
		printk( DEV_ALERT "sig/echo GPIO %d is same as sig/trig\n", hc->echo);
		return 0;
	}

	/* check that echo GPIO is valid */
	if ( !gpio_is_valid(hc->echo) ){
		printk( DEV_ALERT "echo GPIO %d is not valid\n", hc->echo );
		return -EINVAL;
	}

	/* request the GPIO pin */
	if ( gpio_request(hc->echo, DEV_NAME) != 0 ) {
		printk( DEV_ALERT "Unable to request echo GPIO %d\n", hc->echo );
		return -EINVAL;
	}

	/* make GPIO an input */
	if ( gpio_direction_input(hc->echo) != 0 ) {
		printk( DEV_ALERT "Failed to make echo GPIO %d an input\n", hc->echo );
		return -EINVAL;
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

/* Free GPIO */
static void hcsr04_gpio_free( struct hcsr04* hc )
{
	gpio_free(hc->trig);
	if (hc->echo != hc->trig) {
		gpio_free(hc->echo);
	}
	return;
}

/*---------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static const struct of_device_id hcsr04_dt_ids[] = {
	{ .compatible = DEV_NAME, },
	{}
};
MODULE_DEVICE_TABLE(of, hcsr04_dt_ids);
#endif

/* device initialisation function */
static int hcsr04_probe(struct platform_device* pdev)
{
	struct device *dev = &pdev->dev;
	struct hcsr04 *hcsr04;
	struct iio_dev *iio;
	int ret = -1;
	int trig = 0;
	int echo = 0;

	#ifdef CONFIG_OF
	struct device_node *node = dev->of_node;

	if (node) {
		/*
		 * first  gpio: trigger pin
		 * second gpio: echo    pin
		 */
		ret = of_get_gpio(node, 0);
		if (ret >= 0) {
			trig = ret;
			ret = of_get_gpio(node, 1);
			if (ret >= 0) {
				echo = ret;
			} else {
				echo = trig;
			}
		}
	}
	#else
	trig = trig_pin;
	echo = echo_pin;
	#endif

	iio = devm_iio_device_alloc(dev, sizeof(*hcsr04));
	if (!iio) {
		dev_err(dev, "Failed to allocate IIO device\n");
		return -ENOMEM;
	}

	/* fill hcsr04 structure */
	hcsr04 = iio_priv(iio);
	hcsr04->dev = dev;
	hcsr04->trig = trig;
	hcsr04->echo = echo;
	init_waitqueue_head(&hcsr04->wait);
	mutex_init(&hcsr04->lock);

	platform_set_drvdata(pdev, iio);

	iio->name = pdev->name;
	iio->dev.parent = &pdev->dev;
	iio->info = &hcsr04_iio_info;
	iio->modes = INDIO_DIRECT_MODE;
	iio->channels = hcsr04_chan_spec;
	iio->num_channels = ARRAY_SIZE(hcsr04_chan_spec);

	/* Set up the GPIO */
	if ( hcsr04_gpio_init(hcsr04) < 0 )
		return -EINVAL;


	return devm_iio_device_register(dev, iio);
}

/*---------------------------------------------------------------------------*/

/* device removing function */
static int hcsr04_remove(struct platform_device* pdev)
{
	struct iio_dev *iio = platform_get_drvdata(pdev);
	struct hcsr04 *hcsr04 = iio_priv(iio);

	/* Free GPIO */
	hcsr04_gpio_free(hcsr04);

	iio_device_unregister(iio);
	return 0;
}

/*---------------------------------------------------------------------------*/

static struct platform_driver hcsr04_driver = {
	.driver = {
		.name  = DEV_NAME,
		.owner = THIS_MODULE,
		#ifdef CONFIG_OF
		.of_match_table = hcsr04_dt_ids,
		#endif
	},
	.probe  = hcsr04_probe,
	.remove = hcsr04_remove,
};

module_platform_driver(hcsr04_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Ward");
MODULE_AUTHOR("Peter Yang <turmary@126.com>");

/*---------------------------------------------------------------------------*/
