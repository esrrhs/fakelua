import os
import re

directories = ['src', 'include', 'cmd', 'test', 'benchmark']
extensions = ['.h', '.cpp', '.y', '.l']

def is_snake_case(s):
    return s.islower() and '_' in s and not s.startswith('__')

cpp_keywords = {'if', 'while', 'for', 'switch', 'catch', 'sizeof', 'alignof', 'decltype', 'return', 'main', 'std'}
std_functions = {'push_back', 'size', 'empty', 'c_str', 'begin', 'end', 'find', 'insert', 'emplace_back', 'make_shared', 'make_unique', 'to_string', 'cout', 'endl', 'printf', 'fprintf', 'sprintf', 'strcmp', 'strlen'}

names = set()

# Regex to find function definitions/declarations
# e.g. void my_function(int a)
# class my_class
# struct my_struct
# enum my_enum
func_pattern = re.compile(r'\b[a-zA-Z_][a-zA-Z0-9_:<>*&]*\s+([a-z_][a-z0-9_]*)\s*\(')
class_pattern = re.compile(r'\b(?:class|struct|enum(?:\s+class)?)\s+([a-z_][a-z0-9_]*)\b')

for directory in directories:
    for root, _, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                    # Classes/Structs/Enums
                    for m in class_pattern.finditer(content):
                        name = m.group(1)
                        if is_snake_case(name):
                            names.add(name)
                            
                    # Functions
                    # We also need to capture functions like Class::method()
                    method_pattern = re.compile(r'\b[a-zA-Z_][a-zA-Z0-9_:]*::([a-z_][a-z0-9_]*)\s*\(')
                    for m in method_pattern.finditer(content):
                        name = m.group(1)
                        if is_snake_case(name):
                            names.add(name)
                            
                    for m in func_pattern.finditer(content):
                        name = m.group(1)
                        if is_snake_case(name) and name not in cpp_keywords and name not in std_functions:
                            names.add(name)

# Some specific inclusions based on user prompt
prompt_includes = ['fakelua_state', 'var_interface', 'simple_var_impl', 'compile_config', 'compiler', 'vm', 'syntax_tree_node', 'vi_get_type', 'compile_file', 'get_int', 'to_string', 'add_stmt', 'fakelua_newstate', 'native_to_fakelua', 'var_type_to_string', 'var_type', 'syntax_tree_type']
for p in prompt_includes:
    names.add(p)

with open('names.txt', 'w') as f:
    for name in sorted(names):
        f.write(name + '\n')
