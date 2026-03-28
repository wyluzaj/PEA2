#include "writer.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {
    std::string escapeCsv(const std::string& value) {
        bool needsQuotes = false;
        for (char c : value) {
            if (c == ',' || c == '"' || c == '\n') {
                needsQuotes = true;
                break;
            }
        }

        if (!needsQuotes) {
            return value;
        }

        std::string escaped = "\"";
        for (char c : value) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += "\"";
        return escaped;
    }
}

std::string sanitizeFileName(const std::string& input) {
    std::string out = input;
    for (char& c : out) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    return out;
}

void writeBnBSummaryToFile(TSPResult& result) {
    fs::create_directories("results/podglad");

    const std::string safeInstance = sanitizeFileName(result.instance_name);
    const std::string safeAlgo = sanitizeFileName(result.algorithm_name);

    result.summary_file_name =
            "results/podglad/Wynik_" + safeInstance + "_" + safeAlgo + ".txt";

    std::ofstream file(result.summary_file_name);
    if (!file) {
        throw std::runtime_error(
                "Nie mozna utworzyc pliku podgladowego: " + result.summary_file_name
        );
    }

    file << std::fixed << std::setprecision(3);
    file << "Algorytm: " << result.algorithm_name << "\n";
    file << "Instancja: " << result.instance_name << "\n";
    file << "Typ instancji: " << result.instance_type << "\n";
    file << "n: " << result.vertex_count << "\n";
    file << "Czas [ms]: " << result.total_time_ms << "\n";
    file << "Najlepszy znaleziony koszt: " << result.best_cost << "\n";
    file << "Poczatkowe UB: " << result.ub_from_nn << "\n";

    if (!result.best_path_text.empty()) {
        file << "Pelna najlepsza znaleziona sciezka: " << result.best_path_text << "\n";
    }

    file << "Liczba odwiedzonych wezlow: " << result.visited_nodes << "\n";
    file << "Liczba odcietych wezlow: " << result.pruned_nodes << "\n";
    file << "Liczba wygenerowanych dzieci: " << result.generated_nodes << "\n";
    file << "Liczba stanow zapisanych w pamieci: " << result.stored_nodes << "\n";
    file << "Maksymalny rozmiar frontier: " << result.max_frontier_size << "\n";
    file << "Maksymalny rozmiar nodePool: " << result.max_node_pool_size << "\n";
    file << "Brak pamieci: " << (result.memory_exhausted ? "TAK" : "NIE") << "\n";
    file << "Powod zakonczenia: " << result.stop_reason << "\n";
}

void appendBnBResultToCsv(const TSPResult& result, const std::string& csvPath) {
    fs::create_directories("results");

    const bool fileExists = fs::exists(csvPath);
    std::ofstream file(csvPath, std::ios::app);

    if (!file) {
        throw std::runtime_error("Nie mozna otworzyc pliku CSV: " + csvPath);
    }

    if (!fileExists) {
        file << "alg,inst,typ,n,czas[ms],UB,opt,visited,cut,generated,stored,max_frontier,max_pool,memory_exhausted,stop\n";
    }

    file << std::fixed << std::setprecision(3);
    file << escapeCsv(result.algorithm_name) << ','
         << escapeCsv(result.instance_name) << ','
         << escapeCsv(result.instance_type) << ','
         << result.vertex_count << ','
         << result.total_time_ms << ','
         << result.ub_from_nn << ','
         << result.best_cost << ','
         << result.visited_nodes << ','
         << result.pruned_nodes << ','
         << result.generated_nodes << ','
         << result.stored_nodes << ','
         << result.max_frontier_size << ','
         << result.max_node_pool_size << ','
         << (result.memory_exhausted ? 1 : 0) << ','
         << escapeCsv(result.stop_reason)
         << '\n';
}

void printBnBResultToConsole(const TSPResult& result) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Algorytm: " << result.algorithm_name << "\n";
    std::cout << "Instancja: " << result.instance_name << "\n";
    std::cout << "Typ instancji: " << result.instance_type << "\n";
    std::cout << "n: " << result.vertex_count << "\n";
    std::cout << "Czas [ms]: " << result.total_time_ms << "\n";
    std::cout << "Najlepszy znaleziony koszt: " << result.best_cost << "\n";
    std::cout << "Poczatkowe UB: " << result.ub_from_nn << "\n";
    std::cout << "Liczba odwiedzonych wezlow: " << result.visited_nodes << "\n";
    std::cout << "Liczba odcietych wezlow: " << result.pruned_nodes << "\n";
    std::cout << "Liczba wygenerowanych dzieci: " << result.generated_nodes << "\n";
    std::cout << "Liczba stanow zapisanych w pamieci: " << result.stored_nodes << "\n";
    std::cout << "Maksymalny rozmiar frontier: " << result.max_frontier_size << "\n";
    std::cout << "Maksymalny rozmiar nodePool: " << result.max_node_pool_size << "\n";
    std::cout << "Brak pamieci: " << (result.memory_exhausted ? "TAK" : "NIE") << "\n";
    std::cout << "Powod zakonczenia: " << result.stop_reason << "\n";

    if (!result.summary_file_name.empty()) {
        std::cout << "Plik podgladowy: " << result.summary_file_name << "\n";
    }
}