import sys
from PyQt5.QtWidgets import QWidget,QLCDNumber,QSlider,QVBoxLayout,QApplication
from PyQt5.QtCore import Qt

class WinForm(QWidget):
    def __init__(self):
        super().__init__()   
        self.initUI()

    def initUI(self):
        #1 先创建滑块和 LCD 部件
        lcd = QLCDNumber(self)
        slider = QSlider(Qt.Horizontal, self)
        
        slider.setMaximum(1000)
        lcd.display(666)
        slider.setValue(666)

        #2 通过QVboxLayout来设置布局
        vBox = QVBoxLayout()
        vBox.addWidget(lcd)
        vBox.addWidget(slider)

        self.setLayout(vBox)
        #3 valueChanged()是Qslider的一个信号函数，只要slider的值发生改变，它就会发射一个信号，然后通过connect连接信号的接收部件，也就是lcd。
        slider.valueChanged.connect(lcd.display)
       
        slider.setMinimumWidth(200) 
        slider.setFixedHeight(60)
        
        style = "QSlider::groove:horizontal {border:1px solid #999999;height:10px;" \
                            "background-color:#666666;margin:2px 0;}" \
                            "QSlider::handle:horizontal {background-color:#ff0000;border:1px solid #797979;" \
                            "width:50px;margin:-20px;border-radius:25px;}" \
                            
        slider.setStyleSheet(style);
        
        #self.setGeometry(0,0,800,480)
        self.setWindowTitle("拖动滑块LCD显示")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    form = WinForm()
    form.resize(800, 480)
    form.show()                      
    sys.exit(app.exec_())
