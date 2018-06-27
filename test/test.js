
/* Primitive Tests */

primitive("test_primitive_return_5", "integer");
function test_primitive_1() {
    if (test_primitive_return_5() == 5) {
        return 1;
    }
    return 0;
}

primitive("test_primitive_take_2", "void", "integer", "integer");
function test_primitive_2() {
    var a = 100;
    test_primitive_take_2(a, 5);
    if (a == 100) {
        return 1;
    }
    return 0;
}

primitive("test_primitive_take_2_add", "integer", "integer", "integer");
function test_primitive_3() {
    var a = 100;
    var b = test_primitive_take_2_add(a, 5);
    if (b == 105) {
        return 1;
    }
    return 0;
}
