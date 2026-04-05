import os
import re

RENAME_MAP = {
    'type': 'Type',
    'dump': 'Dump',
    'loc': 'Loc',
    'gen_tab': 'GenTab',
    'loc_str': 'LocStr',
    'add_stmt': 'AddStmt',
    'stmts': 'Stmts',
    'set_stmts': 'SetStmts',
    'set_name': 'SetName',
    'set_explist': 'SetExplist',
    'explist': 'Explist',
    'set_varlist': 'SetVarlist',
    'varlist': 'Varlist',
    'add_var': 'AddVar',
    'vars': 'Vars',
    'add_exp': 'AddExp',
    'exps': 'Exps',
    'set_prefixexp': 'SetPrefixexp',
    'get_prefixexp': 'GetPrefixexp',
    'set_exp': 'SetExp',
    'get_exp': 'GetExp',
    'set_type': 'SetType',
    'get_name': 'GetName',
    'get_type': 'GetType',
    'args': 'Args',
    'set_args': 'SetArgs',
    'name': 'Name',
    'set_fieldlist': 'SetFieldlist',
    'fieldlist': 'Fieldlist',
    'add_field': 'AddField',
    'fields': 'Fields',
    'set_key': 'SetKey',
    'set_value': 'SetValue',
    'key': 'Key',
    'value': 'Value',
    'set_label': 'SetLabel',
    'set_block': 'SetBlock',
    'block': 'Block',
    'set_elseiflist': 'SetElseiflist',
    'set_else_block': 'SetElseBlock',
    'elseifs': 'Elseifs',
    'elseblock': 'Elseblock',
    'add_elseif_expr': 'AddElseifExpr',
    'add_elseif_block': 'AddElseifBlock',
    'elseif_size': 'ElseifSize',
    'elseif_exps': 'ElseifExps',
    'elseif_blocks': 'ElseifBlocks',
    'elseif_exp': 'ElseifExp',
    'elseif_block': 'ElseifBlock',
    'set_exp_begin': 'SetExpBegin',
    'set_exp_end': 'SetExpEnd',
    'set_exp_step': 'SetExpStep',
    'exp_begin': 'ExpBegin',
    'exp_end': 'ExpEnd',
    'exp_step': 'ExpStep',
    'set_namelist': 'SetNameList',
    'namelist': 'NameList',
    'add_name': 'AddName',
    'add_attrib': 'AddAttrib',
    'names': 'Names',
    'set_funcname': 'SetFuncName',
    'set_funcbody': 'SetFuncBody',
    'funcname': 'FuncName',
    'funcbody': 'FuncBody',
    'colon_name': 'ColonName',
    'set_colon_name': 'SetColonName',
    'set_parlist': 'SetParlist',
    'parlist': 'Parlist',
    'set_var_params': 'SetVarParams',
    'var_params': 'VarParams',
    'exp_type': 'ExpType',
    'exp_value': 'ExpValue',
    'set_left': 'SetLeft',
    'set_op': 'SetOp',
    'set_right': 'SetRight',
    'left': 'Left',
    'op': 'Op',
    'right': 'Right',
    'get_op': 'GetOp',
    'set_tableconstructor': 'SetTableconstructor',
    'set_string': 'SetString',
    'get_value': 'GetValue',
    # Added some that were missing or inferred
    'prefixexp': 'Prefixexp',
    'tableconstructor': 'Tableconstructor',
    'string': 'String'
}

# Sort by length descending to avoid partial replacements (e.g. set_exp vs set_exp_begin)
SORTED_KEYS = sorted(RENAME_MAP.keys(), key=len, reverse=True)

def refactor_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_content = content
    for old in SORTED_KEYS:
        new = RENAME_MAP[old]
        # Use regex to match old names followed by '(' but only if not part of a larger identifier
        # unless it is preceded by ->, . or ::
        # \b matches word boundaries.
        
        # Match word boundary, then old, then (
        # This handles:
        #  ptr->old(
        #  obj.old(
        #  Class::old(
        #  old(  <- in member function calls inside the same class or declarations
        
        pattern = rf'\b{old}\('
        new_content = re.sub(pattern, f'{new}(', new_content)

    if new_content != content:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Refactored {file_path}")

def main():
    for target_dir in ['src', 'test']:
        for root, dirs, files in os.walk(target_dir):
            for file in files:
                if file.endswith(('.h', '.cpp')):
                    refactor_file(os.path.join(root, file))

if __name__ == "__main__":
    main()
