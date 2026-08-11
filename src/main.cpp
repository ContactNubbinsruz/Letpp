#include "let.hpp"

int main()
{
    let object = obj{ {"key", "value"}, {"age", 15} };
    object["key"] = 5;
    for (let e : object){
        log(e.key());
        log(e.val());
        log("\n");
    }
}
