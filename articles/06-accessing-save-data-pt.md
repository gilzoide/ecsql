# Acessando dados de *save*
> 2025-04-21 | `#ECS #SQL #SQLite #C++ #SaveData` | [*English version*](06-accessing-save-data-en.md)

Não há ferramenta melhor para salvar o progresso dos jogadores do que bancos de dados.
Já temos uma conexão a uma base de dados SQLite, então a utilizaremos também para salvar dados de progresso.


## Acessando a base de dados de *save*
Uma das várias funcionalidades maneiríssimas de SQLite é o suporte a [acessar múltiplas bases de dados numa mesma conexão](https://sqlite.org/lang_attach.html).
Com isso, podemos acessar a base de dados de *save* usando a mesma interface que já utilizamos para acessar os dados de ECS.
Nós podemos inclusive fazer `JOIN` entre dados salvos e dados de ECS, possibilitando a implementação de sistemas que operam em ambas bases de dados ao mesmo tempo.

Para anexar uma base de dados à conexão, basta executarmos um comando SQL `ATTACH`, como por exemplo:
```sql
ATTACH DATABASE 'save.sqlite3' AS save;
```
SQLite vai criar o arquivo da nova base de dados automaticamente se ele ainda não existir.

Após isso, podemos usar o prefixo `save.` para especificar tabelas referentes à base de dados `save` que acabamos de anexar à conexão.
Aqui está um exemplo de tabela com estatísticas de jogo que será salva na base de dados *save*:
```sql
CREATE TABLE save.statistics(
  total_uptime FLOAT DEFAULT 0
);
```

Agora podemos acessar tabelas da base de dados `save` nos nossos sistemas.
Aqui um exemplo de consulta que guarda o tempo de jogo total, persistido entre diferentes sessões de jogo:
```sql
UPDATE save.statistics
SET total_uptime = total_uptime + time.delta
FROM time;
```
Note que `time` é uma tabela que pertence à base de dados de ECS, enquanto `save.statistics` pertence à base de dados de *save*.

> Eu escrevi mais sobre como ECSQL gerencia tempo [aqui](04-tracking-time-pt.md)


## Migrando a base de dados
Conforme a gente desenvolve e lança novas versões do nosso jogo, é comum que precisemos mudar a estrutura dos dados salvos.
Como esses dados são persistidos entre sessões, precisamos de um jeito de modificar a base de dados sem perder os dados existentes.
Não queremos sumir com os dados e fazer os jogadores terem que começar o jogo do zero, não é mesmo.

Uma forma comum de fazer isso é definindo *scripts* de migração da base de dados.
Mantemos uma lista de *scripts* de migração e os executamos na ordem certa quando o jogo começa, desde que não tenham sido executados ainda.
Para sabermos quais *scripts* já foram executados, mantemos um número de versão na própria base de dados.
Aqui está a estrutura de versionamento que ECSQL usa:
```sql
-- A tabela simplesmente guarda a versão atual
CREATE TABLE IF NOT EXISTS save.schema_version(version INTEGER);

-- Usar "OR IGNORE" e especificar o "rowid" aqui garante que a
-- linha é inserida da primeira vez, mas nunca modificada no futuro
INSERT OR IGNORE INTO save.schema_version(rowid, version) VALUES(1, 0);
```

Então por padrão, quando a versão é 0, nenhuma migração foi executada ainda.
No código do jogo a gente então checa qual a versão atual e executa as migrações que estão faltando:
```cpp
int version = world.execute_sql(R"(
    SELECT version
    FROM save.schema_version
)").get<int>();

if (version < 1) {
    // Executa migração 1
    // Por exemplo, cria a tabela "statistics" mencionada acima
    world.execute_sql_script(R"(
        CREATE TABLE save.statistics(
          total_uptime FLOAT DEFAULT 0
        );
        INSERT INTO save.statistics DEFAULT VALUES;

        -- Atualiza a versão atual
        UPDATE save.schema_version
        SET version = 1
    )");
}

if (version < 2) {
    // Executa migração 2
    // Agora queremos manter o número de sessões em "statistics"
    world.execute_sql_script(R"(
        ALTER TABLE save.statistics ADD COLUMN session_count INTEGER DEFAULT 0;

        -- Atualiza a versão atual
        UPDATE save.schema_version
        SET version = 2
    )");
}

// ...
```

Assim como criar e alterar tabelas, podemos também criar e alterar índices, *constraints*, *triggers*, etc.


## Conclusão
Hoje discutimos como salvar dados de progresso do jogador usando uma nova base de dados SQLite e como acessar esses dados em conjunto com os dados de ECS nos nossos sistemas.
Vimos também um jeito simples de gerenciar e aplicar migrações à base de dados de *save*.

Espero que tenha persistido essa informação na sua memória, até a próxima!
