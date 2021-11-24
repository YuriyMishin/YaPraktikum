
#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace tc;

int main (){
    TransportCatalogue transport_catalogue;

    cin >> transport_catalogue;
    cout << transport_catalogue;

    return 0;
}

