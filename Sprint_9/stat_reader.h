#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"


std::ostream& operator << (std::ostream& output, tc::TransportCatalogue& transport_catalogue);

