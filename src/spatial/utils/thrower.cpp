//
// Created by gomkyung2 on 2023/08/14.
//

#include "spatial/utils/thrower.hpp"

#include <stdexcept>

void spatial::utils::throwRuntimeError(const char *what_arg) {
    throw std::runtime_error(what_arg);
}

void spatial::utils::throwOutOfRange(const char *what_arg) {
    throw std::out_of_range(what_arg);
}

void spatial::utils::throwInvalidArgument(const char *what_arg) {
    throw std::invalid_argument(what_arg);
}
