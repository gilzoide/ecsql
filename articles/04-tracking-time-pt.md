# Acompanhando o tempo
Uma coisa que você vai ver em basicamente qualquer motor de jogos é que eles sempre disponibilizam o tempo que passou desde o último quadro (em inglês, *frame*), o famoso *"delta time"*.
Ele é usado para converter entre *frames* e tempo real, possibilitando que jogos executem na mesma velocidade independente de quantos *frames* são processados por segundo.

Por exemplo, se movermos um *sprite* 10 pixels por *frame*, ele estará movendo a uma velocidade de 600 pixels por segundo se o jogo estiver rodando a 60 *frames* por segundo (FPS), mas somente 300 pixels/s se rodando a 30 FPS.
O jogo pode rodar a 60 FPS num dispositivo potente, mas somente a 30 FPS em outro dispositivo mais fraco, precisamos garantir que a experiência de jogo seja a mesma em ambos os casos.
Para garantir uma experiência de jogo consistente, devemos mover o *sprite* usando uma taxa de pixels por segundo ao invés de pixels por *frame*.


## Guardando o tempo em SQL
Vamos guardar o tempo delta e o tempo total de jogo (em inglês, *uptime*) em SQL para que essa informação possa ser utilizada por sistemas.

Vamos criar a tabela *singleton* `time`.
*"Singleton"* nesse caso significa que nessa tabela só vai existir uma única linha.
Sua definição é a seguinte:
```sql
CREATE TABLE time(
  delta,
  uptime
);

-- Aqui inserimos a única linha da tabela
-- O tempo ainda não passou, então zeramos ambos os valores
INSERT INTO time(delta, uptime) VALUES(0, 0);
```

Agora no *loop* do jogo, pegamos o tempo delta atual usando a função `GetFrameTime()` disponível no raylib e passamos esse valor para a seguinte consulta de UPDATE:
```sql
UPDATE time
SET delta = ?1, uptime = uptime + ?1;
```

Note que usamos `?1` para atualizar ambos os valores de `delta` e `uptime`, o que significa que o mesmo valor será usado nos dois casos.
Isso faz com que `uptime` sempre seja incrementado pelo valor de `delta` a cada *frame*, de modo que conterá a soma dos valores de `delta` de todos os *frames*, resultando no tempo total passado desde que o jogo iniciou.


## Usando tempo delta em sistemas
Bora usar como exemplo um sistema de movimento que move o componente `Position` de entidades com velocidade constante, representada pelo componente `LinearVelocity`:
```sql
UPDATE Position
SET
  x = Position.x + LinearVelocity.x,
  y = Position.y + LinearVelocity.y,
  z = Position.z + LinearVelocity.z
FROM LinearVelocity
WHERE Position.entity_id = LinearVelocity.entity_id
```

Para fazer essa consulta usar o tempo delta e mover a entidade com velocidade constante independente do *frame rate*, basta fazermos `JOIN` com a tabela `time` e multiplicar os valores de velocidade por `time.delta`:
```sql
UPDATE Position
SET
  x = Position.x + LinearVelocity.x * time.delta,
  y = Position.y + LinearVelocity.y * time.delta,
  z = Position.z + LinearVelocity.z * time.delta
FROM LinearVelocity JOIN time
WHERE Position.entity_id = LinearVelocity.entity_id
```


## Usando *uptime* em sistemas
Saber qual o tempo total que se passou no jogo é útil para criarmos comportamentos baseados em tempo.

Por exemplo, vamos fazer um sistema que apaga uma entidade após um certo tempo, especificado pelo componente `DeleteAfter`:
```sql
-- CREATE TABLE DeleteAfter(
--   entity_id INTEGER PRIMARY KEY REFERENCES ...,
--   seconds,  -- Tempo em segundos para apagar a entidade
--   inserted_uptime  -- uptime de quando esse componente foi inserido no mundo
-- );

WITH entities_to_delete AS (
  SELECT entity_id
  FROM DeleteAfter JOIN time
  WHERE time.uptime - DeleteAfter.inserted_uptime >= DeleteAfter.seconds
)
DELETE FROM entity
WHERE id IN entities_to_delete;
```


## Conteúdo bônus: forçando a tabela a ser *singleton*
Tá, isso não tem a ver com *loop* de jogo nem tempo delta.
Vou mostrar pra vocês como podemos forçar que ninguém consiga inserir ou apagar linhas na tabela *singleton* `time`.

> Nota: não usei essa técnica no ECSQL, só estou mostrando isso por diversão

O melhor jeito para fazer isso é usando [*triggers*](https://www.sqlite.org/lang_createtrigger.html), que são operações a serem executadas na base de dados quando uma linha é inserida, atualizada ou apagada de uma tabela.
Uma das coisas que conseguimos fazer com *triggers* é abortar a consulta que engatilhou o *trigger*:
```sql
CREATE TRIGGER abort_insert_time_singleton
BEFORE INSERT ON time
BEGIN
  SELECT RAISE(ABORT, 'É proibido inserir linhas na tabela time');
END;

CREATE TRIGGER abort_delete_time_singleton
BEFORE DELETE ON time
BEGIN
  SELECT RAISE(ABORT, 'É proibido apagar linhas da tabela time');
END;
```

Agora tentar inserir ou apagar linhas da tabela `time` falhará com as mensagens que especificamos:
```sql
INSERT INTO time DEFAULT VALUES;
-- Erro: 'É proibido inserir linhas na tabela time'

DELETE FROM time;
-- Erro: 'É proibido apagar linhas da tabela time'
```


## Conclusão
O tempo está sempre passando e precisamos acompanhá-lo no nosso motor de jogos.
Até nosso próximo encontro!
