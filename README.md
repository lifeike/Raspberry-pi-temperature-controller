# RaspberryPi-based termperature controller

#### project work flow（client）
![输入图片说明](https://images.gitee.com/uploads/images/2020/0812/170844_ae00dab3_5695210.png "新新.png")

#### business logic

The client obtains the temperature from the dynamic ds18b20 temperature sensor on the Raspberry Pi at a fixed time, and obtains the real-time time, which is packaged in TLV format and sent to the server. It supports disconnection and reconnection (the data will be stored in the database after the client fails to send data. After connecting to the server, in addition to sending real-time data every time, up to five pieces of data will be obtained from the database for sending), confirm the retransmission mechanism. The database uses a lightweight sqlite3 database, the program operation includes a log system, and the error information of the program operation is updated in real time

#### TLV format
TLV buf:
![TLV](https://images.gitee.com/uploads/images/2020/0812/171543_e037d035_5695210.png "屏幕截图.png")


###### testing tool-TCP Test Tool

# test
### run client only while server disconnected
![输入图片说明](https://images.gitee.com/uploads/images/2020/0813/094511_d71a8009_5695210.png "1.png")

### server running
![输入图片说明](https://images.gitee.com/uploads/images/2020/0813/095048_04b12932_5695210.png "VMSPQZX%FHYT]}6$E31]PNA.png")
Server running here, it will receive 6 pieces of data each time, and after several times, it will only receive once each time, because the server client runs first, and the server is disconnected, the data obtained is saved to the database, and the server is connected After that, every time the client sends the real-time temperature, it will also extract data from the database and send it

![输入图片说明](https://images.gitee.com/uploads/images/2020/0813/095340_18dc07c6_5695210.png "{3H5O7J{_$9}5WTP{@14TDN.png")

### statics
![输入图片说明](https://images.gitee.com/uploads/images/2020/0813/095450_02b9bc64_5695210.png "_Z58M@RWU]`)7Z]Q8UHO2NB.png")
