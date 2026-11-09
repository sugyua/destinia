#include "diagnostic_log.h"
#include <cassert>
#include <string>
int main() {
    DestiniaDiagnosticLog log(2);
    assert(log.text().empty());
    log.add("first"); log.add("second");
    assert(log.text()=="first\nsecond\n");
    log.add("third");
    assert(log.text()=="second\nthird\n");
    DestiniaDiagnosticLog disabled(0);
    disabled.add("discarded");
    assert(disabled.text().empty());
}
