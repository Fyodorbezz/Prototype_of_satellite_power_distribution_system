# Prototype of satellite power distribution system
Prototype of satellite power distribution system that combines power from two different inputs, and provides this power for three "payloads". System monitors different power parameters, protects against short circuit on "payload" side and provides battery backup for "payloads".

![IMG_20220210_171617](https://github.com/Fyodorbezz/Prototype_of_satellite_power_distribution_system/blob/main/Images/IMG_20240315_220943.jpg)


Main PCB picture

![IMG_20220210_171617](https://github.com/Fyodorbezz/Prototype_of_satellite_power_distribution_system/blob/main/Images/IMG_20240221_053256.jpg)


There are two ESP8266 boards with RTC modules that serves as payload simulators. They constantly compute current distance between point in earth and ISS.

![IMG_20220210_171617](https://github.com/Fyodorbezz/Prototype_of_satellite_power_distribution_system/blob/main/Images/IMG_20240221_053357.jpg)


For communication between control station and power distribution system nrf24l01 modules are used. Control station has two displays on which power parameters and warnings are shown. Array of buttons allow to control power switches and manually select power sources for "payloads".

![IMG_20220210_171617](https://github.com/Fyodorbezz/Prototype_of_satellite_power_distribution_system/blob/main/Images/IMG_20240221_062437.jpg)

![IMG_20220210_171617](https://github.com/Fyodorbezz/Prototype_of_satellite_power_distribution_system/blob/main/Images/IMG_20240221_053223.jpg)


YouTube link: https://www.youtube.com/watch?v=Q1t3MN2zMxM
