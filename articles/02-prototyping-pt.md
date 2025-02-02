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

Exemplo:
```cpp
ecsql::Component PositionComponent {
  // nome
  "Position",
  // campos
  {
    "x DEFAULT 0",
    "y DEFAULT 0",
    "z DEFAULT 0",
  },
  // (opcional) SQL adicional
  "",
};
```


## Sistemas
Em ECSQL, sistemas são funções que operam em entidades e componentes do mundo.
Sistemas possuem um nome, uma lista de declarações SQL e a implementação.
A implementação recebe como parâmetro o mundo e uma lista de declarações SQL já preparadas.
Declarações preparadas são mantidas entre chamadas do mesmo sistema, de modo que só gastamos tempo preparando SQL uma vez para cada sistema.

Exemplo:
```cpp
ecsql::System SistemaDesenhaPonto {
  // nome
  "DesenhaPonto",
  // declarações SQL
  {
    R"(
      SELECT
        x, y, z,
        r, g, b, a
      FROM PointTag
        JOIN Position USING(entity_id)
        JOIN Color USING(entity_id)
    )",
  },
  // implementação
  [](ecsql::World& mundo, std::vector<ecsql::PreparedSQL>& sqls_preparados) {
    auto select_pontos_a_desenhar = sqls_preparados[0];
    for (ecsql::SQLRow row : select_pontos_a_desenhar()) {
      auto [posicao, cor] = row.get<Vector3, Color>();
      desenha_ponto(posicao, cor);
    }
  },
};
```


## Sistemas Gancho
Esses sistemas usam [ganchos de *preupdate* do SQLite](https://www.sqlite.org/c3ref/preupdate_blobwrite.html) e são chamados quando componentes são inseridos/atualizados/apagados.
Eles são usados como ponte entre dados do SQL e dados nativos, de modo que dados nativos podem ser criados/atualizados/apagados junto com os dados de SQL correspondentes.

Exemplo:
```cpp
ecsql::HookSystem GanchoPosition {
  // nome do Componente
  "Position"
  // implementação
  [](ecsql::HookType gancho, ecsql::SQLBaseRow& linha_antiga, ecsql::SQLBaseRow& linha_nova) {
    switch (gancho) {
      case ecsql::HookType::OnInsert:
        // Position inserido
        break;

      case ecsql::HookType::OnUpdate:
        // Position atualizado
        break;

      case ecsql::HookType::OnDelete: {
        // Position apagado
        break;
      }
    }
  },
};
```


## Mundo
Finalmente, o mundo ECS.
Mundos contêm uma conexão com a base de dados, uma lista dos sistemas e sistemas gancho registrados.
Existem métodos pra criar entidades, registrar componentes e sistemas, assim como um método `update` que roda todos os sistemas registrados.

Exemplo:
```cpp
ecsql::World mundo;
mundo.register_component(PositionComponent);
mundo.register_system(SistemaDesenhaPonto);
mundo.register_hook_system(GanchoPosition);

while (jogo_esta_rodando()) {
  float delta_time = get_delta_time();
  mundo.update(delta_time);
}
```


## Conclusão
Essa é a arquitetura base do projeto ECSQL.
Nos próximos artigos mostrarei informações mais específicas de problemas e soluções da implementação do ECSQL.
Até a próxima!
