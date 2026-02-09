# DeltaV - Plataforma de Teste de Empuxo

Projeto para medição de empuxo e RPM de drones utilizando ESP32 e Interface Web.

## Estrutura
- **/firmware**: Código C++ para ESP32 (PlatformIO). Lê Célula de Carga (HX711) e LIDAR (RPM).
- **/software**: Dashboard em Python (Flask) que recebe dados via Serial e plota gráficos em tempo real.

## Como Rodar

### 1. Firmware (ESP32)
1. Abra a pasta `firmware` no VSCode com PlatformIO.
2. Faça o upload para o ESP32.

### 2. Software (PC)
1. Instale as dependências:
   `pip install -r software/requirements.txt`
2. Conecte o ESP32 na USB.
3. Verifique a porta COM no arquivo `app.py`.
4. Rode: `python software/app.py`.