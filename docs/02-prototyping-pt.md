# Começando o protótipo: um mundo com entidades, componentes e sistemas
No [artigo anterior](01-ecs-databases-pt.md) foi mostrado como podemos modelar mundos de ECS como bases de dados relacionais.
Hoje vamos passar por algumas escolhas de *design* do protótipo que já comecei.


## Tecnologias base
- C++: uma linguagem que sei usar bem o suficiente e que possui um monte de bibliotecas prontas para desenvolvimento de software/jogos.
  Estou usando o padrão C++20 nesse projeto.
- [CMake](https://cmake.org/) como sistema de construção.
- [SQLite](https://sqlite.org/) como sistema de banco de dados.
- [raylib](https://www.raylib.com/) como biblioteca para fazer o *frontend* do jogo.
  Super fácil de usar, suporta várias plataformas, é uma biblioteca simplesmente maravilhosa.


## Entidades
Em ECSQL, entidades são representadas por IDs numéricos.
Criei o tipo `EntityID` como um *typedef* para o tipo `sqlite3_int64`, que é o tipo do SQLite usado para IDs, e é basicamente isso.


## Componentes
Em ECSQL cada tipo de componente é uma tabela na base de dados.
Criei a classe `Component` que contém o nome do componente, seus campos e um opcionalmente um SQL adicional.
SQLite usa [tipagem flexível](https://sqlite.org/flextypegood.html), então não é necessário especificar o tipo de um campo.
Você pode usar valores "DEFAULT", restrições "CHECK" "NOT NULL" e "REFERENCES", qualquer especificação válida em colunas no SQLite é válido aqui.
O SQL adicional pode ser usado para criar índices, *triggers* e *views*, dentre outros.


## Sistemas
Em ECSQL, sistemas são funções que operam em entidades e componentes do mundo.
Sistemas possuem um nome, uma lista de declarações SQL e a implementação.
A implementação recebe como parâmetro o mundo e uma lista de declarações SQL já preparadas.
Declarações preparadas são mantidas entre chamadas do mesmo sistema, de modo que só gastamos tempo preparando SQL uma vez para cada sistema.


## Sistemas Gancho
Esses sistemas usam [ganchos de *preupdate* do SQLite](https://www.sqlite.org/c3ref/preupdate_blobwrite.html) e são chamados quando componentes são inseridos/atualizados/apagados.
Eles são usados como ponte entre dados do SQL e dados nativos, de modo que dados nativos podem ser criados/atualizados/apagados junto com os dados de SQL correspondentes.


## Mundo
Finalmente, o mundo ECS.
Mundos contêm uma conexão com a base de dados, uma lista dos sistemas e sistemas gancho registrados.
Existem métodos pra criar entidades, registrar componentes e sistemas, assim como um método `update` que roda todos os sistemas registrados.


## Conclusão
Essa é a arquitetura base do projeto ECSQL.
Nos próximos artigos mostrarei informações mais específicas de problemas e soluções da implementação do ECSQL.
Até a próxima!
