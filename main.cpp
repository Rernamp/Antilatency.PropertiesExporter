#include "CLI/CLI.hpp"

int main(int argc, char** argv) {
    CLI::App app{"PropertiesExporter"};

    std::string filename;
    app.add_option("-f,--file", filename, "File name")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    return 0;
}