# Packing Line Automation Project – Mechatronics III

## 📦 Project Overview

This project involves designing and implementing an automated packing line for blocks with dimensions **600×200×300 mm** onto a standard **Euro pallet (1200×800 mm)** in alternating layouts A and B. The line integrates **weighing**, **layer stacking**, and **pallet wrapping** systems. The project is a part of the *Mechatronics III* curriculum.

---

## 🛠️ Line Description

### Flow of the Process:

1. **Initial Conveyor**  
   A forklift loads the blocks onto the first conveyor.

2. **Weighing Station**  
   Blocks are moved onto a second conveyor with a **weighing system**.
   - If the block **weighs less or more than 50 kg**, it is automatically **rejected** onto a side container on the next conveyor.
   - Only blocks with a valid weight continue on the main conveyor.

3. **Robot Palletizing**  
   A robot picks the accepted blocks and stacks them on a **Euro pallet**.
   - The pallet is stacked in **alternating layers (A and B)**.
   - The operator can choose to stack **5, 8, or 10 layers** via the control panel.

4. **Wrapping Station**  
   Once the pallet is full, it is moved to the **wrapping station**.
   - After wrapping is complete, a **signal light** notifies that the product is ready.
   - A forklift operator then moves the pallet to storage.

---

## 🎛️ Operator Control Panel

The control panel includes:
- **Start/Stop** buttons for the entire line.
- **Layer selection**: choose between 5, 8, or 10 layers.
- **Live weight display** for each block.
- **Status indicators** showing the presence of a block on each conveyor segment.

---

## 🧰 Required Project Components

- Electrical control **schematic diagram**
- **PLC wiring** and configuration
- **MCU program**
- **Web HMI visualization** of the operator panel
- **Schematic layout** of the packing line (linear configuration)
- Robot recommendation based on:
  - **Block weight**
  - **Maximum pallet stacking height**

---

## 📐 Design Specifications

- Conveyor dimensions:
  - **Standard conveyors**: 1800×400 mm
  - **Conveyor before and after the Euro pallet**: 1400×1000 mm

- Euro pallet: 1200×800 mm
- Block dimensions: 600×200×300 mm
- Weight tolerance: **50 kg ± 0**

---

## 🤖 Robot Requirements

The selected robot should:
- Be capable of handling **blocks weighing up to 50 kg**
- Reach the maximum pallet stacking height (based on 10 layers)
- Be compatible with the palletizing pattern A/B

---

## 🖥️ Technologies Used

- **espidf framework** 
- **Web development Jquery**
- **Mechatronic system design**
- **CAD tools** for layout and schematics

---

## 📎 License

This project is for **educational purposes** as part of the *Mechatronics III* course.  
All rights reserved to the project authors and their institution.

---

## 📸 Preview

![image](https://github.com/user-attachments/assets/29605b23-445e-42ac-962e-5c633de11b76)


---

## 📫 Contact

If you have any questions, feel free to reach out via the [Issues](https://github.com/your-repo/issues) tab.

---


