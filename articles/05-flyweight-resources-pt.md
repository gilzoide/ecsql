# Recursos externos e o padrão *Flyweight*
> 2025-02-09 | `#ECS #SQLite #C++ #Flyweight` | [*English version*](05-flyweight-resources-en.md)

Um dos problemas que aparece rapidamente ao usar SQLite como base de dados para ECS é: como associar dados nativos usados em C++ com dados de SQL?

Exemplo: para desenhar texturas usando raylib, precisamos de uma instância da estrutura `Texture`.
Mas sistemas consultam os componentes usando SQL, como conseguir a instância `Texture` para desenhar a textura em C++?

Para resolver esse problema, precisamos manter um mapa associativo entre dados de SQL e instâncias nativas.
Cada instância nativa terá um identificador, por exemplo o próprio identificador numérico da entidade associada a ela.
Na implementação dos sistemas, juntamos os dados do SQL com dados nativos usando código C++, consultando esse mapa.
Um `std::map` ou `std::unordered_map` de C++ é o suficiente para implementarmos isso.


## Mapeando SQL e texturas
Vamos pegar como exemplo um componente `Texture` que contém o caminho do arquivo de textura a ser carregado:
```cpp
ecsql::Component TextureComponent {
    "Texture",
    {
        "asset_path TEXT",
    }
};
```

Vamos criar o mapa que vai guardar instâncias nativas de `Texture` para cada entidade:
```cpp
std::unordered_map<ecsql::EntityId, Texture> TextureMap;
```

Na implementação do sistema, consultamos esse mapa de texturas usando o identificador que pegamos direto do SQL:
```cpp
ecsql::System DrawTextureAtOrigin {
    // nome do sistema
    "DrawTextureAtOrigin",
    // consultas SQL
    {
        "SELECT entity_id FROM Texture",
    },
    // implementação
    [](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sqls) {
        auto select_texture = prepared_sqls[0];
        for (ecsql::SQLRow row : select_texture()) {
            auto entity_id = row.get<ecsql::EntityId>();
            auto it = TextureMap.find(entity_id);
            if (it != TextureMap.end()) {
                Texture texture = it->second();
                DrawTexture(texture, 0, 0, WHITE);
            }
        }
    },
};
```

Tudo o que falta agora é de fato carregar a textura no mapa quando necessário e descarregá-la quando ela não for mais necessária.
Usaremos um sistema gancho para isso, de modo que a textura será carregada automaticamente quando uma linha for inserida na tabela do componente `Texture` e descarregada quando a linha for apagada:
```cpp
ecsql::HookSystem TextureMapHook {
    // nome do componente
    "Texture"
    // implementação
    [](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
        switch (hook) {
            case ecsql::HookType::OnInsert: {
                auto [entity_id, asset_path] = new_row.get<ecsql::EntityId, const char *>();
                Texture texture = LoadTexture(asset_path);
                TextureMap.emplace(entity_id, texture);
                break;
            }

            case ecsql::HookType::OnDelete: {
                auto entity_id = old_row.get<ecsql::EntityId>();
                auto it = TextureMap.find(entity_id);
                if (it != TextureMap.end()) {
                    UnloadTexture(it->second());
                    TextureMap.erase(it);
                }
                break;
            }
        }
    },
};
```

Agora nosso sistema `DrawTextureAtOrigin` deve conseguir desenhar texturas com sucesso.


## Reutilizando recursos duplicados
Uma coisa que você vai notar é que se usarmos o mesmo arquivo de textura para várias entidades, a textura será carregada várias vezes e ficaremos com várias instâncias duplicadas, uma para cada entidade que possuir um componente `Texture`.
Ao invés de carregar uma mesma textura várias vezes, queremos manter uma única instância nativa para cada textura carregada e compartilhá-la entre todas as entidades que utilizarem o mesmo caminho de arquivo.
Essa ideia de compartilhar dados de objetos similares referenciando um outro objeto em comum para reduzir uso de memória é o cerne do padrão de projeto [*Flyweight*](https://pt.wikipedia.org/wiki/Flyweight).

Agora, ao invés de mapearmos entidades a instâncias de `Texture`, vamos mapeá-las baseado no caminho do arquivo.
Se 100 entidades utilizarem uma textura com o mesmo caminho, todas elas compartilharão a mesma instância nativa.
Isso não só ajuda a reduzir o uso de memória, como ajuda a reduzir o uso de GPU ao possibilitar o *batching* de desenho (não vou entrar em detalhes sobre isso nesse artigo).

Pra ter certeza que vamos descarregar a textura somente quando nenhuma das entidades precisar dela mais, vamos manter uma contagem de referências para cada textura.
Quando houver 0 entidades referenciando uma textura e inserirmos uma nova linha na tabela `Texture`, carregaremos a instância nativa da textura e marcaremos a contagem de referência como 1.
Conforme mais linhas são adicionadas, continuamos incrementando a contagem de referência sem carregar a textura novamente.
Conforme as linhas são apagadas, decrementamos a contagem de referência.
No momento em que apagarmos a última linha e a contagem de referência for de 1 para 0, finalmente descarregaremos a textura.

Utilizarei meu próprio projeto [flyweight.hpp](https://github.com/gilzoide/flyweight.hpp) como implementação de *Flyweight*, já que ele suporta contagem de referência automática.
Então ao invés de manter um `std::unordered_map<ecsql::Entity_id, Texture>`, agora teremos um *Flyweight* com contagem de referência que mapeia caminho de arquivos (`std::string`) para texturas nativas:
```cpp
flyweight::flyweight_refcounted<std::string, Texture> TextureFlyweight {
    // Função chamada quando a contagem for de 0 para 1: carrega a textura
    [](const std::string& asset_path) {
        return LoadTexture(asset_path.c_str());
    },
    // Função chamada quando a contagem for de 1 para 0: descarrega a textura
    [](Texture& texture) {
        UnloadTexture(texture);
    }
}
```

Nosso sistema gancho agora fica assim:
```cpp
ecsql::HookSystem TextureMapHook {
    // nome do componente
    "Texture"
    // implementação
    [](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
        switch (hook) {
            case ecsql::HookType::OnInsert: {
                auto asset_path = new_row.get<std::string>(1);
                // `get` incrementa a contagem de referências
                // A textura será carregada somente quando contar de 0 para 1
                TextureFlyweight.get(asset_path);
                break;
            }

            case ecsql::HookType::OnDelete: {
                auto asset_path = old_row.get<std::string>(1);
                // `release` decrementa a contagem de referências
                // A textura será decarregada somente quando contar de 1 para 0
                TextureFlyweight.release(asset_path);
                break;
            }
        }
    },
};
```

E o sistema `DrawTextureAtOrigin` agora ficou assim:
```cpp
ecsql::System DrawTextureAtOrigin {
    // nome do sistema
    "DrawTextureAtOrigin",
    // consultas SQL
    {
        "SELECT asset_path FROM Texture",
    },
    // implementação
    [](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sqls) {
        auto select_texture = prepared_sqls[0];
        for (ecsql::SQLRow row : select_texture()) {
            auto asset_path = row.get<std::string>();
            // `peek` retorna um ponteiro para a textura, se estiver carregada
            // Como temos o sistema gancho, ela sempre estará carregada!
            Texture *texture = TextureFlyweight.peek(asset_path);
            DrawTexture(*texture, 0, 0, WHITE);
        }
    },
};
```


## Conclusão
Nem tudo o que é necessário para rodar o jogo pode ser armazenado na base de dados, e tá tudo bem.
Podemos sempre juntar os dados de SQL com dados nativos do C++ usando mapas associativos.

Também vimos como evitar recursos nativos duplicados utilizando o padrão de projeto *Flyweight*, que é uma técnica bem comum em motores de jogos.

E é isso, espero que tenham gostado!

Abraço \o/
