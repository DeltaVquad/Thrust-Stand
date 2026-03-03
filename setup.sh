#!/bin/bash
# Script de Setup DeltaV - Otimizado para Linux
echo "Iniciando Protocolo de Configuração DeltaV..."

# 1. Instalação de dependências do sistema
sudo apt update && sudo apt install python3-full python3-venv curl -y

# 2. Correção da URL das regras UDEV (Caminho atualizado 2024/2025)
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules

# 3. Aplicação de regras e permissões
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -a -G dialout $USER

# 4. Configuração do Ambiente Virtual Python
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
    echo "Ambiente virtual criado."
fi

source .venv/bin/activate
pip install -r Software/requirements.txt

echo "========================================================="
echo "Setup concluído! Reinicie sua sessão (Logout/Login) para"
echo "que as permissões de grupo (dialout) entrem em vigor."
echo "========================================================="