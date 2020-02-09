//=============================================================================			
//init script for i.MX6UL DDR3			
//=============================================================================			
// Revision History			
// v01			
//=============================================================================			
			
wait = on			
//=============================================================================			
// Disable	WDOG		
//=============================================================================			
setmem /16	0x020bc000 =	0x30	
			
//=============================================================================			
// Enable all clocks (they are disabled by ROM code)			
//=============================================================================			
setmem /32	0x020c4068 =	0xffffffff	
setmem /32	0x020c406c =	0xffffffff	
setmem /32	0x020c4070 =	0xffffffff	
setmem /32	0x020c4074 =	0xffffffff	
setmem /32	0x020c4078 =	0xffffffff	
setmem /32	0x020c407c =	0xffffffff	
setmem /32	0x020c4080 =	0xffffffff	
			
			
//=============================================================================			
// IOMUX			
//=============================================================================			
//DDR IO TYPE:			
setmem /32	0x020e04b4 =	0x000C0000	// IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE 
setmem /32	0x020e04ac =	0x00000000	// IOMUXC_SW_PAD_CTL_GRP_DDRPKE 
			
//CLOCK:			
setmem /32	0x020e027c =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0
			
//ADDRESS:			
setmem /32	0x020e0250 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS
setmem /32	0x020e024c =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS
setmem /32	0x020e0490 =	0x00000028	// IOMUXC_SW_PAD_CTL_GRP_ADDDS 
			
//Control:			
setmem /32	0x020e0288 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET
setmem /32	0x020e0270 =	0x00000000	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS
setmem /32	0x020e0260 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0
setmem /32	0x020e0264 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1
setmem /32	0x020e04a0 =	0x00000028	// IOMUXC_SW_PAD_CTL_GRP_CTLDS 
			
//Data Strobes:			
setmem /32	0x020e0494 =	0x00020000	// IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL 
setmem /32	0x020e0280 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0 
setmem /32	0x020e0284 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1 
			
//Data:			
setmem /32	0x020e04b0 =	0x00020000	// IOMUXC_SW_PAD_CTL_GRP_DDRMODE
setmem /32	0x020e0498 =	0x00000028	// IOMUXC_SW_PAD_CTL_GRP_B0DS 
setmem /32	0x020e04a4 =	0x00000028	// IOMUXC_SW_PAD_CTL_GRP_B1DS 
			
setmem /32	0x020e0244 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0
setmem /32	0x020e0248 =	0x00000028	// IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1
			
			
//=============================================================================			
// DDR Controller Registers			
//=============================================================================			
// Manufacturer:	Micron		
// Device Part Number:	MT41K256M16HA-125		
// Clock Freq.: 	400MHz		
// Density per CS in Gb: 	4		
// Chip Selects used:	1		
// Number of Banks:	8		
// Row address:    	15		
// Column address: 	10		
// Data bus width	16		
//=============================================================================			
setmem /32	0x021b001c =	0x00008000	// MMDC0_MDSCR, set the Configuration request bit during MMDC set up
			
//=============================================================================			
// Calibration setup.			
//=============================================================================			
setmem /32	0x021b0800 =	0xA1390003	// DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic HW ZQ calibration.
			
// For target board, may need to run write leveling calibration to fine tune these settings.			
setmem /32	0x021b080c  =	 0x00050003	
			
//Read DQS Gating calibration			
setmem /32	0x021b083c =	0x013C0140	// MPDGCTRL0 PHY0
			
//Read calibration			
setmem /32	0x021b0848 =	0x40402E30	// MPRDDLCTL PHY0
			
//Write calibration                     			
setmem /32	0x021b0850 =	0x4040322E	// MPWRDLCTL PHY0
			
//read data bit delay: (3 is the reccommended default value, although out of reset value is 0)			
setmem /32	0x021b081c =	0x33333333	// MMDC_MPRDDQBY0DL
setmem /32	0x021b0820 =	0x33333333	// MMDC_MPRDDQBY1DL
			
//write data bit delay: 			
setmem /32	0x021b082c =	0xF3333333	// MMDC_MPWRDQBY0DL
setmem /32	0x021b0830 =	0xF3333333	// MMDC_MPWRDQBY1DL
			
//DQS&CLK Duty Cycle			
setmem /32	0x021b08c0 =	0x00921012	// [MMDC_MPDCCR] MMDC Duty Cycle Control Register
			
// Complete calibration by forced measurement:                  			
setmem /32	0x021b08b8 = 	0x00000800	// DDR_PHY_P0_MPMUR0, frc_msr
//=============================================================================			
// Calibration setup end			
//=============================================================================			
			
//MMDC init: 			
setmem /32	0x021b0004 =	0x0002002D	// MMDC0_MDPDC
setmem /32	0x021b0008 =	0x1B333030	// MMDC0_MDOTC
setmem /32	0x021b000c =	0x676B52F3	// MMDC0_MDCFG0
setmem /32	0x021b0010 =	0xB66D0B63	// MMDC0_MDCFG1
setmem /32	0x021b0014 =	0x01FF00DB	// MMDC0_MDCFG2
			
//MDMISC: RALAT kept to the high level of 5. 			
//MDMISC: consider reducing RALAT if your 528MHz board design allow that. Lower RALAT benefits: 			
//a. better operation at low frequency, for LPDDR2 freq < 100MHz, change RALAT to 3			
//b. Small performence improvment 			
setmem /32	0x021b0018 =	0x00211740	// MMDC0_MDMISC
setmem /32	0x021b001c =	0x00008000	// MMDC0_MDSCR, set the Configuration request bit during MMDC set up
setmem /32	0x021b002c =	0x000026D2	// MMDC0_MDRWD
setmem /32	0x021b0030 =	0x006B1023	// MMDC0_MDOR
setmem /32	0x021b0040 =	0x0000004F	// Chan0 CS0_END 
setmem /32	0x021b0000 =	0x84180000	// MMDC0_MDCTL
			
setmem /32	0x021b0890 =	0x00400a38	// MPPDCMPR2
			
//Mode register writes                 			
setmem /32	0x021b001c =	0x02008032	// MMDC0_MDSCR, MR2 write, CS0
setmem /32	0x021b001c =	0x00008033	// MMDC0_MDSCR, MR3 write, CS0
setmem /32	0x021b001c =	0x00048031	// MMDC0_MDSCR, MR1 write, CS0
setmem /32	0x021b001c =	0x15208030	// MMDC0_MDSCR, MR0write, CS0
setmem /32	0x021b001c =	0x04008040	// MMDC0_MDSCR, ZQ calibration command sent to device on CS0
			
//setmem /32	0x021b001c =	0x0200803A	// MMDC0_MDSCR, MR2 write, CS1
//setmem /32	0x021b001c =	0x0000803B	// MMDC0_MDSCR, MR3 write, CS1
//setmem /32	0x021b001c =	0x00048039	// MMDC0_MDSCR, MR1 write, CS1
//setmem /32	0x021b001c =	0x15208038	// MMDC0_MDSCR, MR0write, CS1
//setmem /32	0x021b001c =	0x04008048	// MMDC0_MDSCR, ZQ calibration command sent to device on CS1
			
setmem /32	0x021b0020 =	0x00007800	// MMDC0_MDREF
			
setmem /32	0x021b0818 =	0x00000227	// DDR_PHY_P0_MPODTCTRL
			
setmem /32	0x021b0004 =	0x0002556D	// MMDC0_MDPDC now SDCTL power down enabled
			
setmem /32	0x021b0404 =	0x00011006	// MMDC0_MAPSR ADOPT power down enabled, MMDC will enter automatically to self-refresh while the number of idle cycle reached.
			
setmem /32	0x021b001c =	0x00000000	// MMDC0_MDSCR, clear this register (especially the configuration bit as initialization is complete)
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			

