# **PSoC 4 Embedded Systems Lab Projects**

### *Computer Systems and Embedded Programming Course*

## 🇬🇧 English Version

### **Overview**

This repository contains a structured set of laboratory projects developed during the *Computer Systems and Networks* course. All implementations target the **Cypress PSoC 4 platform**, using the **CY8CKIT-042 Pioneer Kit**.

The goal of this repository is to demonstrate a **step-by-step progression in embedded systems development**, starting from low-level GPIO manipulation and advancing toward **interrupt-driven architectures, communication interfaces, and real-time control systems**.

---

### **Laboratory Modules**

#### **Lab 1: GPIO Control & Active-Low Logic**

* Configuration of digital inputs using *resistive pull-up*.
* Output configuration with *strong drive mode*.
* Button-controlled RGB LED behavior.
* Introduction to active-low signal logic.

---

#### **Lab 3: Matrix Keypad & Finite State Machines**

* Implementation of **4x3 matrix keypad scanning algorithm**.
* Software **debouncing techniques**.
* FSM-based input handling using `switch-case`.
* UART integration for:

  * Debug logging
  * Password validation system

---

#### **Lab 4: Shift Registers (74HC595)**

* Manual SPI implementation via **bit-banging**.
* Serial-to-parallel data conversion.
* Driving **7-segment displays** using shift registers.
* Timing-sensitive signal generation.

---

#### **Lab 5: Timers, Interrupts & Multiplexing**

* Configuration of **hardware timers**.
* Writing **Interrupt Service Routines (ISR)**.
* Implementation of **dynamic indication (multiplexing)** for multi-digit displays.
* Signal validation using a **logic analyzer**.

---

### **Development Environment**

* **IDE:** PSoC Creator 4.4
* **Compiler:** ARM GCC 5.4.1
* **MCU:** CY8C4245AXI-483 (ARM Cortex-M0)
* **Debugging Tools:**

  * Saleae Logic Analyzer
  * PuTTY / Tera Term

---

## 🇺🇦 Українська версія

### **Опис**

Цей репозиторій містить набір лабораторних робіт, виконаних у рамках курсу **«Комп'ютерні системи та мережі»**. Усі проєкти реалізовані на базі **Cypress PSoC 4** із використанням плати **CY8CKIT-042 Pioneer Kit**.

Основна мета — показати **поетапне освоєння embedded-розробки**, починаючи з базових операцій із GPIO і закінчуючи **перериваннями, протоколами обміну та системами реального часу**.

---

### **Лабораторні роботи**

#### **Лаб. 1: GPIO та інверсна логіка**

* Налаштування цифрових входів (*resistive pull-up*).
* Конфігурація виходів (*strong drive*).
* Керування RGB-світлодіодом через кнопку.
* Робота з active-low сигналами.

---

#### **Лаб. 3: Матрична клавіатура та скінченні автомати**

* Сканування клавіатури **4x3**.
* Програмний антидребізг.
* Обробка станів через FSM (`switch-case`).
* UART для:

  * логування
  * перевірки паролів

---

#### **Лаб. 4: Зсувні регістри (74HC595)**

* Реалізація SPI через **bit-banging**.
* Перетворення послідовних даних у паралельні.
* Керування **7-сегментними індикаторами**.
* Контроль таймінгів сигналів.

---

#### **Лаб. 5: Таймери, переривання та мультиплексування**

* Налаштування апаратних таймерів.
* Обробники переривань (**ISR**).
* Динамічна індикація (мультиплексування).
* Перевірка сигналів через логічний аналізатор.

---

### **Інструменти**

* **IDE:** PSoC Creator 4.4
* **Компілятор:** ARM GCC 5.4.1
* **Мікроконтролер:** CY8C4245AXI-483 (Cortex-M0)
* **Налагодження:**

  * Saleae Logic Analyzer
  * PuTTY / Tera Term
