#ifndef TABLE_HPP
#define TABLE_HPP

#include <iostream>
#include <unordered_map>
#include <list>
#include <map>

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
    int relative_address = -1;
};

// SCOPE
// This is a tree structure that stores the scope of each variable  (e.g. global, function, block)
class Table{
public:
    int scope_size;
    std::unordered_map <std::string, metadata> symbol_table;
    Table* parent;
    std::list <Table*> children;

    // Iterator to keep track of the current child
    std::list<Table*>::iterator currentChild;

    Table(Table* parent = nullptr){
        this->scope_size = 0;
        this->parent = parent;
        this->currentChild = children.end();
    }

    // Method to add a new scope
    Table* scopeIn() {
        Table* newScope = new Table(this);
        children.push_back(newScope);
        return newScope;
    }

    // Method to move to the outer scope
    Table* scopeOut() {
        if (parent != nullptr) {
            return parent;
        } else {
            // Already at the global scope or no parent scope exists.
            throw std::runtime_error("No outer scope to move to.");
        }
    }

    // Method to traverse in
    Table* traverseIN(){
       if (currentChild == children.end()) {
            if (!children.empty()) {
                currentChild = children.begin();  // Start with the first child
            } else {
                throw std::runtime_error("No child scope to traverse into.");
            }
        } else {
            // Move to the next child
            ++currentChild;
            if (currentChild == children.end()) {
                throw std::runtime_error("No more child scopes to traverse into.");
            }
        }
        return *currentChild;
    }

    // Method to traverse out
    Table* traverseOUT(){
        if (parent != nullptr) {
            // Reset the current child iterator
            currentChild = children.end();
            return parent;
        } else {
            throw std::runtime_error("No parent scope to move back to.");
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
            // if(data.type == data_type::UNKNOWN){
            //     data.address = -1;
            //     symbol_table[name] = data;
            // }else{
            data.address = scope_size;
            scope_size += data.size;
            symbol_table[name] = data;
            // }
        } else {
            throw std::runtime_error("Variable already exists: " + name);
        }
    }

    // Method to get a variable
    metadata& getVariable(const std::string& name) {
        for (Table* current = this; current != nullptr; current = current->parent) {
            auto it = current->symbol_table.find(name);
            if (it != current->symbol_table.end()) {
                return it->second;
            }
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    // Method to set the relative address of a variable
    void set_relativeAddress(std::string name, int relativeAddress) {
        for (Table* current = this; current != nullptr; current = current->parent) {
            auto it = current->symbol_table.find(name);
            if (it != current->symbol_table.end()) {
                it->second.relative_address = relativeAddress;
                return;
            }
        }
        throw std::runtime_error("Variable not found for setting relative address: " + name);
    }

    // Debugging purposes only
    void printSymbolTable(int indent = 0) const {
        if(indent == 0) std::cout << "Symbol Table:" << std::endl;
        std::string indentStr(indent, ' ');  // Create an indentation string

        std::cout << indentStr << "Scope Size: " << scope_size << std::endl;

        for (const auto& pair : symbol_table) {
            std::cout << indentStr << pair.first << ": Type=" << static_cast<int>(pair.second.type) 
                    << ", Size=" << pair.second.size << ", Address=" << pair.second.address << std::endl;
        }

        for (Table* child : children) {
            child->printSymbolTable(indent + 4);  // Increase indent for nested scopes
            std::cout << std::endl;
        }
    }
    
};

// GLOBAL SYMBOL TABLE
Table* SYMBOL_TABLE = new Table(nullptr);

// Map to hold string literals and their corresponding labels
std::map<std::string, std::string> stringLiterals;

// String literal counter
int stringLiteralCounter = 0;

#endif