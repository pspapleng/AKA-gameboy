<!--
*** Thanks for checking out this README Template. If you have a suggestion that would
*** make this better, please fork the repo and create a pull request or simply open
*** an issue with the tag "enhancement".
*** Thanks again! Now go create something AMAZING! :D
-->

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/pspapleng/ArduinoBoy">
    <img src="https://raw.githubusercontent.com/othneildrew/Best-README-Template/master/images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">ArduinoBoy
</h3>

  <p align="center">
   enjoy with ArduinoBoy :)
    <br />
    <a href="https://www.youtube.com/watch?v=eQNCPSPgyeM&list=LLOwSX5axlQ6ORKyyI-jCeLQ&index=2&t=10s"><strong>View Demonstrate Video »</strong></a>
    <br />
    <br />
    <a href="https://github.com/pspapleng/ArduinoBoy">Explore The Doc</a>
    ·
    <a href="https://github.com/pspapleng/ArduinoBoy/issues">Report Bug</a>
    ·
    <a href="https://github.com/pspapleng/ArduinoBoy/issues">Request Feature</a>
  </p>
</p>

<!-- TABLE OF CONTENTS -->

## Table of Contents

- [About the Project](#about-the-project)
  - [Abstract](#Abstract)
  - [Objective](#Objective)
  - [Built With](#built-with)
- [Equipment](#Equipment)
- [Features](#Features)
- [Flowchart](#Flowchart)
- [Demonstrate Video](#Demonstrate-Video)
- [Contact](#contact)

<!-- ABOUT THE PROJECT -->

# About The Project

[![Product Name Screen Shot][product-screenshot]](https://raw.githubusercontent.com/pspapleng/GoodDeal-Project/master/img/1576522078513.jpg)

### Abstract

โครงงานนี้เป็นโครงงานเกี่ยวกับการทำเครื่องเล่นเกมแบบพกพาหรือที่เรียกกันว่าเกมบอย โดยมีวัตถุประสงค์เพื่อใช้เล่นสำหรับผ่อนคลายความเครียด และฝึกทักษะการคิดวิเคราะห์และแก้ไขปัญหา โดยเทคโนโลยีที่นำมาใช้ในโครงงานนี้คือ Arduino UNO + WiFi board โดยใช้ร่วมกับโมดูลจอยสติ้กเพื่อควบคุมทิศทางในการเล่นเกม มีการใช้โมดูลจอสี TFT LCD เพื่อแสดงผลต่างๆเกี่ยวกับเกม และมีการต่อแบตเตอรี่ไว้กับตัวบอร์ดทำให้เราสามารถพกพาเครื่องเล่นเกมนี้ไปใช้งานนอกสถานที่ได้ ซึ่งเกมในเครื่องเล่นเกมของพวกเรานั้นจะพัฒนามาจากเกมงูโดยจะมีการปรับลูกเล่นต่างๆให้มีเพิ่มมากขึ้นและแตกต่างจากรูปแบบที่มีอยู่เดิม โดยเราจะนำภาษาซีมาประยุกต์ใช้ในการเขียนเกมลงในตัว Arduino board

Keywords : เครื่องเล่นเกม, เกมบอย, arduino, microcontroller

### Objective

- ใช้สำหรับผ่อนคลายความเครียด
- ใช้ฝึกทักษะการคิดวิเคราะห์และแก้ไขปัญหา

### Built With

- C language
- Arduino UNO + WiFi board

<!-- Equipment -->

# Equipment

- Arduino UNO+WiFi R3 ATmega328P+ESP8266 (32Mb memory)
  เป็นบอร์ด Arduino UNO รุ่นใหม่ที่มีการรวม Wi-Fi Module เข้าไปบนบอร์ด โดยใช้ MCU เป็น ATmega328P และใช้ชิป Wi-Fi เป็น ESP8266 จาก Espressif

- โมดูลจอสี TFT LCD 2.0 นิ้ว 176x220
  Product Introduction

* Size:2.0 inch SPI serial bus
* Resolution:176x220
* Driving IC:ILI9225

- JoyStick Shield expansion board for Arduino

* จอยสติ้กสำหรับ Arduino พร้อมช่องรองรับการเชื่อมต่อแบบไร้สาย NRF24L01

<!-- Features -->

# Features

- มีการสุ่มกำแพงขึ้นมาพร้อมกับอาหาร เมื่องูกินอาหารแล้วกำแพงอันนั้นจะเปลี่ยนเป็นอาหารแทนแล้วทำการสุ่มกำแพงใหม่ขึ้นมา
- สามารถบังคับทิศทางของงูได้โดยใช้จอยสติ้ก
- ถ้างูชนกำแพงที่ขอบหรือกำแพงที่สุ่มมาจะถือว่าจบเกม
- มีการนับคะแนนจากการกินอาหาร โดยกินอาหาร 1 ครั้ง เท่ากับ 10 คะแนน เมื่อจบเกมจะมีการแสดงผลคะแนนขึ้นที่หน้าจอ

<!-- Flowchart -->

# Flowchart

<!-- Link youtube -->

# Demonstrate Video

- [ArduinoBoy](https://www.youtube.com/watch?v=3QBNH6m6Ctc&feature=youtu.be)

<!-- CONTACT -->

## Contact

62070029 นางสาว จิตติภรณ์ จักรวิวัฒนากุล - [@DevJubjang](https://github.com/DevJubjang)

62070036 นางสาว จุฬาลักษณ์ เอื้อสกุล - [@ChulalakU](https://github.com/ChulalakU)

62070039 นางสาว ชญานิน อินทร์สุข - [@pspapleng](https://github.com/pspapleng)

62070227 นาย อิทธิ์ศรา ยันต์เจริญ - [@lanzelotx](https://github.com/lanzelotx)

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->

[contributors-shield]: https://img.shields.io/github/contributors/pspapleng/GoodDeal-Project.svg?style=flat-square
[contributors-url]: https://github.com/pspapleng/GoodDeal-Project/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/pspapleng/GoodDeal-Project.svg?style=flat-square
[forks-url]: https://github.com/pspapleng/GoodDeal-Project/network/members
[stars-shield]: https://img.shields.io/github/stars/pspapleng/GoodDeal-Project.svg?style=flat-square
[stars-url]: https://github.com/pspapleng/GoodDeal-Project/stargazers
[issues-shield]: https://img.shields.io/github/issues/pspapleng/GoodDeal-Project.svg?style=flat-square
[issues-url]: https://github.com/pspapleng/GoodDeal-Project/issues
[license-shield]: https://img.shields.io/github/license/pspapleng/GoodDeal-Project.svg?style=flat-square
[license-url]: https://github.com/pspapleng/GoodDeal-Project/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=flat-square&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/othneildrew
[product-screenshot]: https://raw.githubusercontent.com/pspapleng/GoodDeal-Project/master/img/1576522078513.jpg
[design-screenshot]: https://raw.githubusercontent.com/pspapleng/GoodDeal-Project/master/img/S__5062662.jpg
[design-card-screenshot]: https://raw.githubusercontent.com/pspapleng/GoodDeal-Project/master/img/S__5062660.jpg
