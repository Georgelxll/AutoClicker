
# Mouse Mover
* O Mouse Mover é um aplicativo para Windows que permite automatizar movimentos e cliques do mouse com base em coordenadas específicas. O programa inclui funcionalidades para capturar coordenadas do mouse, iniciar e parar a execução, e configurar a velocidade e repetição dos movimentos.

**Funcionalidades**

Captura de Coordenadas: Permite capturar a posição atual do mouse e salvá-la em uma lista.
Início e Parada: Inicia a movimentação automática do mouse para as coordenadas capturadas e realiza cliques. O processo pode ser parado a qualquer momento.
Repetição: Possui uma opção para repetir o movimento e os cliques continuamente.
Velocidade: Ajuste do intervalo entre os cliques através de um controle deslizante.
Importação e Exportação: Permite importar e exportar coordenadas de e para arquivos de texto.

**Requisitos**

Sistema Operacional: Windows
Compilador: Microsoft Visual Studio ou qualquer compilador compatível com Windows API
Compilação e Execução
Compilar o Código:

Abra o código-fonte no Microsoft Visual Studio.
Compile o projeto. O código usa a Windows API e deve ser compilado com suporte a recursos comuns (commctrl.h) e caixas de diálogo (commdlg.h).

**Executar o Programa:**

Após a compilação, execute o arquivo gerado (MouseMover.exe).

* Uso
  
**Captura de Coordenadas:**

Clique no botão "Coordenadas" na interface do programa.
Pressione F5 para capturar a posição atual do mouse e adicioná-la à lista de coordenadas.

**Iniciar a Execução:**

Clique no botão "Iniciar" para começar a mover o mouse para as coordenadas capturadas e realizar cliques com o botão direito do mouse.

**Parar a Execução:**

Pressione F4 para parar a execução do movimento do mouse.

**Repetição:**

Clique no menu "Configurações" e selecione "Repetir" para ativar ou desativar a repetição dos movimentos e cliques.

**Configurar Velocidade:**

Clique em "Velocidade" no menu "Configurações" para abrir uma caixa de diálogo onde você pode ajustar o intervalo entre os cliques.

**Importar e Exportar Coordenadas:**

Utilize as opções "Exportar" e "Importar" no menu "Configurações" para salvar e carregar coordenadas de um arquivo de texto.

* Estrutura do Código
  
WindowProc: Função de callback para tratar mensagens da janela, como comandos e eventos de teclado.
ShowSpeedDialog: Função para exibir uma caixa de diálogo de configuração de velocidade.
SaveCoordinatesToFile: Função para salvar coordenadas em um arquivo de texto.
LoadCoordinatesFromFile: Função para carregar coordenadas de um arquivo de texto.

**Licença**

Este projeto é de código aberto. Sinta-se à vontade para usar e modificar conforme necessário.
