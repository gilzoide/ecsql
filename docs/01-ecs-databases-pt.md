> Nota: esse documento assume que você tenha conhecimentos básicos de SQL

## ECS: o básico
Entidade Componente Sistema, ou ECS, é um padrão de projeto que provê reusabilidade de código ao separar a lógica de negócio dos dados, comumente usado em jogos.
Projetos ECS são compostos de:
- Entidades: objetos que vivem em um mundo, representados por identificadores únicos.
- Componentes: estruturas de dados atrelados a entidades.
  Uma entidade pode ter zero ou mais componentes, que podem ser adicionados ou removidos dela dinamicamente.
  Por exemplo, um componente "Posição" usado por objetos no espaço 3D poderiam ter 3 propriedades numéricas "x", "y" e "z".
- Sistemas: funções que implementam a lógica de negócio, aplicadas seletivamente a entidades que possuam um conjunto específico de componentes.
  Por exemplo, um sistema "Mover" que move entidades no espaço 3D poderia ser aplicado somente a entidades que possuem pelo menos o componente "Posição" e o componente "Velocidade".
- Mundo: um conjunto de dados de ECS, composto pelas entidades e seus componentes.

Para informações mais detalhadas sobre conceitos do padrão ECS, recomendo a letura do [ECS FAQ](https://www.flecs.dev/ecs-faq/) (em Inglês).


## ECS e bases de dados relacionais
Um mundo de ECS pode ser enxergado como uma base de dados, que mantém os dados das entidades existentes e seus componentes.
O mundo é consultado pelos sistemas de modo que somente as entidades que possuem o conjunto certo de componentes sejam processadas, usando uma espécie de operação *JOIN* entre diferentes componentes.
Componentes podem ser adicionados e removidos de entidades em uma relação um-pra-um (1:1) ou um-pra-muitos (1:N).

Pensando nessa analogia de mundos ECS como bases de dados, e se a gente implementasse um *framework* de ECS utilizando um banco de dados SQL, por exemplo [SQLite](https://sqlite.org)?

Os pros:
- Com SQL, conseguimos não só consultar um conjunto específico de componentes, mas podemos filtrá-los baseado nos valores das suas colunas.
  Por exemplo, podemos consultar por valores de enumeração específicos em componentes que implementam máquinas de estados.
- Podemos utilizar uma base de dados em memória ou operar diretamente em arquivos.
  Usar arquivos é potencialmente mais lento, já que requer operações de escrita e leitura no sistema de arquivos, mas é uma funcionalidade que pode ser útil.
  Por exemplo, salvar o estado do mundo em disco permite que vejamos o estado final da aplicação após um *crash*.
- Podemos salvar e carregar todos os dados de um mundo utilizando a [API de backup do SQLite](https://www.sqlite.org/backup.html), implementando facilmente a funcionalidade de *quick save* e *quick load*.
- Já que os dados do mundo são operados usando SQL, podemos implementar sistemas em diferentes linguagens de programação e todas elas conseguem acessar os mesmos dados: basta cada uma manter uma conexão com a mesma base de dados compartilhada.
  Isso é especialmente útil para desenvolver funcionalidades específicas de plataforma e sistemas de scripting.

Os contras:
- Mesmo que evitemos a leitura e escrita em disco usando uma base de dados em memória, consultar os dados usando SQL deve ser mais lento do que as estruturas de dados especializadas que outros *frameworks* utilizam, talvez ordens de magnitude mais lento.
  Um dos benefícios do padrão ECS é possibilitar um *layout* dos dados que otimiza o uso da memória *cache* do processador, de modo que iterar e alterar componentes seja bem rápido.
- Alguns tipos de dados, como ponteiros e outras estruturas nativas, não possuem uma representação direta em SQL.
- Usuários precisam saber SQL para aproveitar essa implementação ao máximo.

Mesmo que uma implementação de ECS utilizando SQL provavelmente fique mais lenta do que as implementações especializadas que existem por aí, SQLite é muito rápido no geral e provavelmente vai ser rápido o suficiente para muitos projetos de jogos, então quero experimentar essa ideia e ver o quão longe ela vai.
Esse *framework* experimental de ECS baseado em SQLite se chamará **ECSQL**.


## Representando entidades em SQL
Em ECS, entidades são representadas por identificadores únicos, normalmente números inteiros.
Para representar entidades em SQL, basta criarmos uma tabela `entity` que mantém as entidades existentes, representadas por seu ID numérico:
```sql
CREATE TABLE entity (
  id INTEGER PRIMARY KEY
);
```


## Representando componentes em SQL
Componentes são estruturas de dados que podem ser atreladas a entidades.
Todos os componentes precisam estar associados a uma entidade existente.
Quando a entidade for apagada, seus componentes devem ser apagados junto com ela.

Para mim, o melhor jeito de representar componentes em SQL é criando uma tabela para cada um.
- Cada instância de um componente é identificada pelo ID da entidade a qual ela pertence.
- Para consultar um conjunto de componentes para usarmos em um sistema, tudo o que precisamos é fazer `JOIN` em todas essas tabelas utilizando o ID da entidade.
- Para ter certeza que componentes são apagados junto com a entidade a qual pertence, basta utilizarmos uma chave estrangeira com `ON DELETE CASCADE` e SQLite vai apagar os componentes automaticamente quando sua entidade for apagada.

A base para qualquer componente pode ser definida da seguinte forma:
```sql
CREATE TABLE component (
  -- ID da entidade a qual pertence
  -- "PRIMARY KEY": componentes são identificados pelo ID da sua entidade
  -- "REFERENCES entity(id)": chave estrangeira, relacionamento 1:1
  -- "ON DELETE CASCADE": componente será apagado quando sua entidade for apagada
  entity_id INTEGER PRIMARY KEY REFERENCES entity(id) ON DELETE CASCADE
);
```

Por exemplo, o componente "Posição" para objetos 3D poderia ser definido assim:
```sql
CREATE TABLE position (
  entity_id INTEGER PRIMARY KEY REFERENCES entity(id) ON DELETE CASCADE,

  -- Eixos de posição em 3D, todos com valor padrão 0
  x DEFAULT 0,
  y DEFAULT 0,
  z DEFAULT 0
);
```

Como um outro exemplo, o componente "Velocidade" poderia ser definido assim:
```sql
CREATE TABLE velocity (
  entity_id INTEGER PRIMARY KEY REFERENCES entity(id) ON DELETE CASCADE,

  -- Valor de velocidade, em m/s
  value DEFAULT 0
);
```


## Consultando componentes em sistemas
Os sistemas em si não são armazenados na base de dados SQLile, mas utilizam SQL para consultar os componentes.
Vamos usar o sistema "Mover" como exemplo mais uma vez, que é aplicado a entidades que possuem tanto o componente "Posição" quanto o componente "Velocidade" descritos acima.
Podemos iterar sobre as entidades que possuem ambos os componentes utilizando a seguinte consulta:
```sql
SELECT 
  entity_id,
  position.x, position.y, position.z,
  velocity.value
FROM position
JOIN velocity USING(entity_id);
```

Podemos também suportar facilmente sistemas onde alguns componentes são obrigatórios, mas outros componentes são opcionais.
Para isso, basta trocarmos o `JOIN` por `LEFT JOIN` ou `RIGHT JOIN`, de modo que a consulta retorna `NULL` pros valores dos componentes que não estão presentes na entidade.


## Conclusão
Vimos uma breve introdução ao padrão de projeto ECS e como mundos ECS podem ser modelados usando bases de dado relacionais e SQL.

No próximo artigo, começaremos a implementar nosso *framework* experimental **ECSQL**.
Vejo vocês lá!
