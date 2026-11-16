#include "static_analyzer.hpp"
#include "node.hpp"

StaticAnalyzer::StaticAnalyzer() : has_error(false), error("") {
    symbol_table.emplace_back();
}

void StaticAnalyzer::analyze(Prog::Ptr &prog) {
    for (Decl::Ptr &decl : prog->decls) {
        switch (decl->decl_type) {
        case DeclType::Type:
        case DeclType::Var:
        case DeclType::Func:
        }
    }
}