## Control Actuator Binar

Pentru un actuator binar (ex.: bec de iluminare prin releu sau alt actuator convenit cu mentorul), se va realiza o aplicație pentru MCU care:

- interpretează comenzi de control de la utilizator prin STDIO (serial sau keypad)
- controlează actuatorul
- afișează starea curentă pe un display LCD

Aplicația va implementa următoarele funcționalități:

### 🔹 Control actuator binar (Actuator Control)
- Primește comenzi **ON/OFF** de la utilizator prin STDIO (serial/tastatură)
- Aplică **debouncing**
- Controlează starea actuatorului (releu, LED) la o recurență configurabilă (ex.: 50–100 ms)
- Expune starea actuatorului printr-o interfață internă  
  *(ex.: `actuator_get_state()`)*

### 🔹 Condiționare semnal (Signal Conditioning)
- Aplică operații de preprocesare:
  - saturare
  - debouncing software pentru eliminarea comutărilor false
  - validare persistentă a stării
- Task-ul poate rula la aceeași recurență sau una derivată

### 🔹 Afișare și raportare (Display & Reporting)
- La o recurență mai mică (ex.: 500 ms):
  - afișează pe LCD sau STDIO (`printf`) un raport structurat
  - include starea actuatorului
  - poate include eventuale alerte