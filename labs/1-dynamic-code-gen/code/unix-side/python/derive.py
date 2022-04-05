import arm_none_eabi_config as config
# e.g. import risc_v_config as config ??

import bitwise
import instruction

import re

def solve_instruction(asm, template, names, options, regex):
    """Solves a template and prints the resulting assembly.
    
    Args:
        asm (Assembler): The assembler to use.
        template (str): The template to solve.
        options (dict): The options to choose from for the template arguments.
        regex (str): The regular expression to use to match the template arguments.

    Returns:
        Instruction: The instruction that was solved from the given information.
    """

    matches = list(re.finditer(regex, template))

    # should encode opcode bits
    instr_unchanged = None

    # dictionary mapping {argname: offset}
    args = {}

    if len(matches) != len(names):
        raise RuntimeError(f"Template `{template}` has {len(matches)} arguments but {len(names)} names, {names}.")

    # iterate through each matched location in the template
    for match, name in zip(matches, names):

        # for the current location
        always_0 = None
        always_1 = None

        for option in options[match.group(0)]:
            # insert current option into selected location
            filled_template = template[:match.start()] + option + template[match.end():]
            for fmt in options:
                # set everything else to default value
                filled_template = filled_template.replace(fmt, options[fmt][0])
            
            # generate binary for instruction
            instr = asm.assemble(filled_template)

            # setup always_0 and always_1 to match length of instruction
            if instr_unchanged is None:
                instr_unchanged = bytes([0xFF for i in range(len(instr))]) 
            if always_0 is None:
                always_0 = bytes([0xFF for i in range(len(instr))])
                always_1 = bytes([0xFF for i in range(len(instr))])


            # print(filled_template, format(int.from_bytes(instr, byteorder="big"), 'x'))
            always_0 = bitwise.AND(always_0, bitwise.NOT(instr))
            always_1 = bitwise.AND(always_1, instr)

        both = bitwise.AND(always_0, always_1) 
        if (bitwise.to_int(both) != 0):
            raise RuntimeError(f"Impossible, some bits are both always 0 and 1: {both}")

        unchanged = bitwise.OR(always_0, always_1) # should be the instruction encoding
        my_bits = bitwise.NOT(unchanged)
        args[name] = bitwise.FFS(my_bits) # assumes contiguous fields...


        instr_unchanged = bitwise.AND(instr_unchanged, unchanged) # update opcode bits

    name = template.split(' ')[0] # op name is first word in assembly template
    opcode = bitwise.AND(instr_unchanged, instr)
    return instruction.Instruction(asm.arch(), name, opcode, args)

def main():
    # Assembler object that gives .assemble and .cleanup methods
    asm = config.asm
    # Dictionary of {assembly template: list of argument names}
    templates = config.templates
    # Dictionary of {argument: list of possible values}
    options = config.options
    # Regular expression to match arguments in templates
    regex = config.regex

    for template, names in templates.items():
        solved = solve_instruction(asm, template, names, options, regex)
        print(solved.to_c_function())


    asm.cleanup()

if __name__ == "__main__":
    main()