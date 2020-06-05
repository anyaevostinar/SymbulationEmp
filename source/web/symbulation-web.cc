#include <iostream>

#include "web/web.h"

namespace UI = emp::web;
UI::Document doc("emp_base");

int main(){
    doc << "<h1>It's Symbulation Time!</h1>";
}