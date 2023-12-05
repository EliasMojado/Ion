#ifndef TABLE_HPP
#define TABLE_HPP

#include <iostream>
#include <unordered_map>
#include <list>

// DATA TYPES
enum class data_type{
    INTEGER, CHAR, STRING, FLOAT, BOOLEAN, UNKNOWN
};

// METADATA
// This is a struct that stores the metadata of each variable
struct metadata{
    data_type type;
    bool is_function;
    int size;
    int address;
};

// SCOPE
// This is a tree structure that stores the scope of each variable  (e.g. global, function, block)
class Table{
public:
    int scope_size;
    std::unordered_map <std::string, metadata> symbol_table;
    Table* parent;
    std::list <Table*> children;

    Table(Table* parent = nullptr, int offset = 0){
        this->parent = parent;
        scope_size = offset;
    }

    // Method to add a new scope
    Table* scopeIn() {
        Table* newScope = new Table(this, scope_size);
        children.push_back(newScope);
        return newScope;
    }

    // Method to move to the outer scope
    Table* scopeOut() {
        if (parent != nullptr) {
            parent->scope_size = this->scope_size;
            return parent;
        } else {
            // Already at the global scope or no parent scope exists.
            throw std::runtime_error("No outer scope to move to.");
        }
    }

    // Helper method to check if a variable exists in any parent scope
    bool isVariableExists(const std::string& name) {
        for (Table* current = this; current != nullptr; current = current->parent) {
            if (current->symbol_table.find(name) != current->symbol_table.end()) {
                return true;
            }
        }
        return false;
    }

    // Method to add a variable
    void addSymbol(const std::string& name, metadata& data) {
        if (!isVariableExists(name)) {
            data.address = scope_size;
            scope_size += data.size;
            symbol_table[name] = data;
        } else {
            throw std::runtime_error("Variable already exists: " + name);
        }
    }

    // Method to get a variable
    metadata getVariable(const std::string& name) {
        for (Table* current = this; current != nullptr; current = current->parent) {
            auto it = current->symbol_table.find(name);
            if (it != current->symbol_table.end()) {
                return it->second;
            }
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    // Debugging purposes only
    void printSymbolTable(int indent = 0) const {
        if(indent == 0) std::cout << "Symbol Table:" << std::endl;
        std::string indentStr(indent, ' ');  // Create an indentation string
        for (const auto& pair : symbol_table) {
            std::cout << indentStr << pair.first << ": Type=" << static_cast<int>(pair.second.type) 
                    << ", Size=" << pair.second.size << ", Address=" << pair.second.address << std::endl;
        }
        for (Table* child : children) {
            child->printSymbolTable(indent + 4);  // Increase indent for nested scopes
        }
    }
    
};

// GLOBAL SYMBOL TABLE
Table* SYMBOL_TABLE = new Table(nullptr, 0);


#endif