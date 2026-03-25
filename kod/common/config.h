#ifndef PEA2_CONFIG_H
#define PEA2_CONFIG_H
#pragma once

namespace Config {

    // ===== LIMIT CZASU =====
    // 15 minut = 900 sekund
    constexpr double DEFAULT_TIME_LIMIT_SECONDS = 900.0;

    // ===== STARTOWY WIERZCHOŁEK =====
    // dla użytkownika = 1, wewnętrznie = 0
    constexpr int START_VERTEX = 0;

}

#endif //PEA2_CONFIG_H
