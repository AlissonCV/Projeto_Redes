# Projeto_Redes
Repositório criado para upload do código de acionamento de dois pistões com o auxilio de 5 arduinos.
- Arduino 1: Código de leitura do acionamento do botão e envio via ethernet da mensagem de status 'botão acionado' (Arduino 2);
- Arduino 2 (Escravo): Código de leitura da posição do pistão A (Sensor 1s0 e 1s1), recebimento via ethernet do status 'botão acionado' e envio via I2C (Mestre 3/Escravo 2) do status 'posição do pistão A' e 'botão acionado';
- Arduino 3 (Mestre): Código de leitura recebimento via I2C (Mestre 3/Escravo 2) do status 'posição do pistão A' e 'botão acionado', e recebimento via Xbee do status 'posição do pistão B'. Onde será realizado uma logica de acionamento de acordo com a sequência ('A+{A-//B+}A+{A-//B-}') para acionamento do rele da valvula solenoide do pistão A (Arduino 3) ou envio bia Xbee do comando de acioamento do rele da valvula solenoide do pistão B (Arduino 5);
- Arduino 4 (Escravo): Código de leitura da posição do pistão B (Sensor 2s0 e 2s1) e envio via I2C (Mestre 5/Escravo 4) do status 'posição do pistão B';
- Arduino 5 (Mestre): Código de leitura recebimento via I2C (Mestre 5/Escravo 4) do status 'posição do pistão B' e 'botão acionado', e recebimento via Xbee do acionamento do rele da valvula solenoide do pistão B (Arduino 5);

'''diff
Obs.: Os demais arquivos contém dados não atualizados. Código está incorreto.
'''