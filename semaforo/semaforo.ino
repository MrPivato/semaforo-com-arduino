// constantes (notacao usada para pinos geralmente)
#define semaforoVerde 5 //Define semaforoVerde como pino 5
#define semaforoAmarelo 6 //Define semaforoAmarelo como pino 6
#define semaforoVermelho 7 //Define semaforoVermelho como pino 7
#define passagemVerde 8 //Define passagemVerde como pino 8
#define passagemVermelho 9 //Define passagemVermelho como pino 9
#define botao 10 //Define botao como pino 10
#define alarme 11 //Define alarme como pino 11
#define sensor 0 //Define sensor como pino 0

// constantes do sensor
#define tamanhoAmostra 4
#define sensibilidade 5

// numero de batidas por minuto
const float BPMnormal = 60.00;
const float BPMmedio = 145.00;
const float BPMalto = 175.00;
const float BPMmaximo = 185.00;

void setup()
{

  //Configura os pinos como saída
  pinMode(semaforoVerde, OUTPUT);
  pinMode(semaforoAmarelo, OUTPUT);
  pinMode(semaforoVermelho, OUTPUT);
  pinMode(passagemVerde, OUTPUT);
  pinMode(passagemVermelho, OUTPUT);
  pinMode(alarme, OUTPUT);

  pinMode(botao, INPUT_PULLUP); //Configura o pino 10 como entrada

  Serial.begin(9600);

}

void loop()
{

  while (digitalRead(botao)) //Enquanto o botao do pedestre não for pressionado
  {
    digitalWrite(semaforoVerde, HIGH); //Liga o LED Verde do carro
    digitalWrite(passagemVermelho, HIGH); //Liga o LED Vermelho do pedestre
  }

  // variaveis
  float leituras[tamanhoAmostra], soma;
  long int agora, ptr;
  float ultima, leitor, inicio;
  float primeira, segunda, terceira, antes, valorObtido;
  bool subindo;
  int numBatida;
  int n;
  long int ultimaBatida;
  int c = 0;

  delay(2700); // espera 2.7 segundos para a medicao

  for (int i = 0; i < tamanhoAmostra; i++)
    leituras[i] = 0;

  soma = 0;
  ptr = 0;

  while (c < 300)
  {
    /*
       faz uma media das medicoes do sensor
       leva 20 ms, assim eliminando uma parte
       da interferencia da luz (50 Hz)
    */

    n = 0;
    inicio = millis();
    leitor = 0.;

    do
    {
      leitor += analogRead(sensor);
      n++;
      agora = millis();
    }
    while (agora < inicio + 20);  // passados os 20 ms

    leitor /= n;  // faz media

    /*
       adiciona a ultima medicao p/ um array
       e diminui a medicao mais antiga p/
       manter uma soma das medicoes
    */
    soma -= leituras[ptr];
    soma += leitor;
    leituras[ptr] = leitor;
    ultima = soma / tamanhoAmostra;
    // assim o ultimo valor guarda a media dos valores

    /*
       checa se existe uma batida:
       o valor da ultima medicao
       esta maior
    */
    if (ultima > antes)
    {
      numBatida++;
      if (!subindo && numBatida > sensibilidade)
      {
        /*
           detectamos uma curva ascendente
           entao recordamos os tempos assim os
           armazenando em variaveis para uma media
           e a variavel subindo nos impede de
           de detectarmos o mesmo batimento
        */
        subindo = true;
        primeira = millis() - ultimaBatida;
        ultimaBatida = millis();

        // calcula a media conforme as ultimas 3 medidas
        valorObtido = 60000. / (0.4 * primeira + 0.3 * segunda + 0.3 * terceira);

        terceira = segunda;
        segunda = primeira;

      }
    }
    else
    {
      // a curva desce
      subindo = false;
      numBatida = 0;
    }

    antes = ultima;
    ptr++;
    ptr %= tamanhoAmostra;
    c++;
  }

  Serial.print(valorObtido);
  Serial.print('\n');

  if (valorObtido < BPMnormal)
  {
    // batimentos muito baixos
    // algo de errado aconteceu, então não fazemos nada
    goto fim;
  }
  else if (valorObtido > BPMmaximo)
  {
    // batimentos muito altos
    // algo de errado aconteceu, então não fazemos nada
    goto fim;
  }

  digitalWrite(semaforoVerde, LOW);
  digitalWrite(semaforoAmarelo, HIGH);
  delay(3000); //Aguarda 3 segundos no amarelo
  digitalWrite(semaforoAmarelo, LOW);
  digitalWrite(semaforoVermelho, HIGH);
  delay(300);
  digitalWrite(passagemVermelho, LOW);
  digitalWrite(passagemVerde, HIGH);

  // checagem de batimentos
  if (valorObtido >= BPMnormal && valorObtido < BPMmedio)
  {
    delay(30000); //Aguarda 30 segundos
  }
  else if (valorObtido >= BPMmedio && valorObtido < BPMalto)
  {
    delay(20000); //Aguarda 20 segundos
  }
  else if (valorObtido >= BPMalto && valorObtido <= BPMmaximo)
  {
    delay(15000); //Aguarda 15 segundos
  }

  digitalWrite(passagemVerde, LOW);
  delay(200);

  //Pisca o vermelho do pedestre e o alarme
  for (int i = 0 ; i < 15 ; i++) // Pisca 8 vezes
  {
    // inverte estados
    digitalWrite(passagemVermelho, !(digitalRead(passagemVermelho)));
    digitalWrite(alarme, !(digitalRead(alarme)));
    delay(500);
  }

  digitalWrite(alarme, LOW); // desliga o alarme, caso ligado
  delay(200);
  digitalWrite(semaforoVermelho, LOW);

fim:
  ;
}
