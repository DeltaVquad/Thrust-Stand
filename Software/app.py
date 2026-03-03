import serial
import serial.tools.list_ports
import json
import time
from flask import Flask, render_template
from flask_socketio import SocketIO

# --- CONFIGURAÇÕES DINÂMICAS ---
def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for p in ports:
        # Busca pelo chip Silicon Labs do seu lsusb (10c4:ea60)
        if "10c4:ea60" in p.hwid or "CP210" in p.description:
            return p.device
    return '/dev/ttyUSB0' # Fallback para Linux padrão

SERIAL_PORT = find_esp32_port()
BAUD_RATE = 115200
THRESHOLD = 0.001
NEGATIVE_LIMIT = -0.4

app = Flask(__name__)
app.config['SECRET_KEY'] = 'deltav_secret'
socketio = SocketIO(app, cors_allowed_origins='*', async_mode='eventlet')

thread = None
last_status = {'msg': 'Aguardando Servidor...', 'color': '#6c757d'} 
last_valid_thrust = 0.0
in_warning_state = False

def broadcast_status(msg, color):
    global last_status
    if last_status['msg'] != msg:
        print(f"STATUS: {msg}")
    last_status = {'msg': msg, 'color': color}
    socketio.emit('bridge_status', last_status)

def background_thread():
    global last_valid_thrust, in_warning_state
    
    while True:
        try:
            broadcast_status(f"Conectando em {SERIAL_PORT}...", "#ffc107")
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            ser.reset_input_buffer()
            broadcast_status("Sistema Online: ESP32 Conectado", "#28a745")
            
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    # LOG DE DIAGNÓSTICO: Verifique se isso aparece no terminal!
                    if line:
                        print(f"DEBUG RAW: {line}")
                    
                    if line.startswith('{') and line.endswith('}'):
                        try:
                            data = json.loads(line)
                            raw_thrust = float(data.get('thrust', 0))
                            
                            # Filtro de Zona Morta
                            if abs(raw_thrust - last_valid_thrust) >= THRESHOLD:
                                last_valid_thrust = raw_thrust
                            
                            data['thrust'] = last_valid_thrust
                            
                            # Segurança de limite negativo
                            if last_valid_thrust < NEGATIVE_LIMIT:
                                if not in_warning_state:
                                    broadcast_status(f"ALERTA: IMPULSO NEGATIVO ({last_valid_thrust} kgf)", "#ff0000")
                                    in_warning_state = True
                            else:
                                if in_warning_state:
                                    broadcast_status("Sistema Online: ESP32 Conectado", "#28a745")
                                    in_warning_state = False
                            
                            socketio.emit('update_sensor', data)
                        except json.JSONDecodeError:
                            continue
                socketio.sleep(0.01)

        except (serial.SerialException, OSError):
            broadcast_status("ERRO: ESP32 Desconectado! Tentando reconectar...", "#dc3545")
            socketio.sleep(2)

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def connect():
    global thread
    socketio.emit('bridge_status', last_status)
    if thread is None:
        thread = socketio.start_background_task(background_thread)

if __name__ == '__main__':
    print(f"Iniciando DeltaV | Porta: {SERIAL_PORT}")
    socketio.run(app, debug=True, port=5000, host='0.0.0.0')