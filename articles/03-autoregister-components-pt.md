# Registrando componentes automaticamente
> 2025-02-01 | `#ECS #C++ #GlobalVariables #LinkedList` | [*English version*](03-autoregister-components-en.md)

Depois de prototipar um pouco, percebi que registrar componentes manualmente é uma tarefa muito entediante.
Eu estava criando todos os componentes como variáveis globais no C++, daí registrado elas uma a uma usando `mundo.register_component(componente)` na função `main` da aplicação.

Para simplificar, criei uma lista ligada global que contém referências a cada uma das instâncias de componente:
```cpp
class Component {
private:
    // 1. Declare a lista global
    inline static std::forward_list<Component *> lista_global;

public:
    // 2. Insira cada componente à lista em seu construtor
    Component(...) {
      lista_global.push_front(this);
    }

    // 3. Crie o método "foreach" que aplica uma função às instâncias
    template<typename Fn>
    static void foreach(Fn&& f) {
        for (auto it : lista_global) {
            f(it);
        }
    }
};
```

Daí registrei todos os componentes usando a função `foreach`:
```cpp
ecsql::Component::foreach([&](ecsql::Component *componente) {
    mundo.register_component(*componente);
});
```

E é isso!
Agora, pra criar compomentes, basta definí-los num arquivo C++, e serão registrados automaticamente!
```cpp
// Exemplo: meu_componente.cpp
ecsql::Component MeuComponente {
    "MeuComponente",
    {
        "campo1",
        "campo2",
    },
};

// Agora MeuComponente está na lista global e será registrado automaticamente
```


## Sistemas
Uma coisa importante sobre variáveis globais e C++ é que elas são construídas na ordem em que são declaradas dentro de uma mesma unidade de tradução (cada arquivo .cpp é uma unidade de tradução na maioria das vezes), mas a ordem é indefinida entre unidades de tradução diferentes.

Isso significa que só podemos usar essa estratégia se a order de construção dos objetos não for relevante.
Componentes podem ser registrados basicamente em qualquer ordem, então é tranquilo usar uma lista global para eles.
Para sistemas, por outro lado, pode ser interessante definirmos a ordem de registro manualmente, já que a ordem de execução deles dentro de um mesmo *frame* é a mesma ordem de registro.
Por causa disso, não usei essa técnica com sistemas, que atualmente são registrados um por um.


## Sistemas gancho
Sistemas gancho, por outro lado, também podem ser registrados em qualquer ordem.
Não acho que vou precisar preocupar com a ordem de execução de sistemas gancho nesse projeto, então usei a mesma tática da lista global com eles também.


## Conclusão
É isso, trabalhei nisso para ter menos trabalho no futuro.
Nóis \o/
