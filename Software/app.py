import serial
import json
import time
from flask import Flask, render_template
from flask_socketio import SocketIO

# --- CONFIGURAÇÕES ---
SERIAL_PORT = 'COM3'   # <--- CONFIRA SUA PORTA
BAUD_RATE = 115200
THRESHOLD = 0.001      # Filtro de oscilação (Deadband)
NEGATIVE_LIMIT = -0.4  # <--- LIMITE DE ALERTA NEGATIVO (kgf)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'deltav_secret'
socketio = SocketIO(app, cors_allowed_origins='*', async_mode=None)

thread = None
last_status = {'msg': 'Aguardando Servidor...', 'color': '#6c757d'} 
last_valid_thrust = 0.0

# Variável para controlar se estamos em modo de alerta (para não ficar mandando msg repetida)
in_warning_state = False

def broadcast_status(msg, color):
    global last_status
    # Só imprime no terminal se a mensagem mudou (evita spam no log)
    if last_status['msg'] != msg:
        print(f"STATUS: {msg}")
    
    last_status = {'msg': msg, 'color': color}
    socketio.emit('bridge_status', last_status)

def background_thread():
    global last_valid_thrust, in_warning_state
    
    broadcast_status(f"Procurando {SERIAL_PORT}...", "#ffc107")
    
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            ser.reset_input_buffer()
            broadcast_status("Sistema Online: ESP32 Conectado", "#28a745")
            in_warning_state = False # Resetar estado de alerta ao conectar
            
            while True:
                if ser.in_waiting > 0:
                    try:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        
                        if line.startswith('{') and line.endswith('}'):
                            data = json.loads(line)
                            
                            # --- 1. FILTRO DE ZONA MORTA ---
                            raw_thrust = float(data.get('thrust', 0))
                            
                            if abs(raw_thrust - last_valid_thrust) >= THRESHOLD:
                                last_valid_thrust = raw_thrust
                            
                            data['thrust'] = last_valid_thrust
                            
                            # --- 2. VERIFICAÇÃO DE SEGURANÇA (NEGATIVO) ---
                            # Se o valor for muito negativo (ex: -0.45)
                            if last_valid_thrust < NEGATIVE_LIMIT:
                                if not in_warning_state:
                                    broadcast_status(f"ALERTA: IMPULSO NEGATIVO ({last_valid_thrust} kgf)", "#ff0000")
                                    in_warning_state = True
                            
                            # Se o valor estiver normal (ex: -0.1 ou 0.5)
                            else:
                                if in_warning_state:
                                    # Voltou ao normal, restaura mensagem verde
                                    broadcast_status("Sistema Online: ESP32 Conectado", "#28a745")
                                    in_warning_state = False
                            
                            socketio.emit('update_sensor', data)
                    except:
                        pass
                        
                socketio.sleep(0.01)

        except serial.SerialException:
            broadcast_status("ERRO: ESP32 Desconectado! Reconectando...", "#dc3545")
            in_warning_state = False # Reseta estado
            try:
                ser.close()
            except:
                pass
            socketio.sleep(2)

        except Exception as e:
            broadcast_status(f"Erro Interno: {str(e)}", "#dc3545")
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
    print(f"Iniciando DeltaV | Limite Negativo: {NEGATIVE_LIMIT} kgf")
    socketio.run(app, debug=True, port=5000)