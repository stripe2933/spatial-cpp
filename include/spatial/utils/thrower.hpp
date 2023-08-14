//
// Created by gomkyung2 on 2023/08/14.
//

#ifndef SPATIAL_THROWER_HPP
#define SPATIAL_THROWER_HPP

namespace spatial::utils{
    [[noreturn]] void throwRuntimeError(const char *what_arg);
    [[noreturn]] void throwInvalidArgument(const char *what_arg);
    [[noreturn]] void throwOutOfRange(const char *what_arg);
};

#endif //SPATIAL_THROWER_HPP
