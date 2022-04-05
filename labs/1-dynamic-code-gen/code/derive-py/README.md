# derive-py

Python reimplementation of DERIVE. Runs on standard `python3` with no additional libraries.

## Usage

To derive various arm-none-eabi instructions:
```
python3 derive.py arm_none_eabi_config
```

You can create a new config file using `arm_none_eabi_config.py` as an example. It just needs to define:

- `asm`: an instance of a subclass of `assembler.Assembler`
- `templates`: a list of tuples (instr_name, instr_template, argument_names)
- `options`: dictionary mapping each possible identifier in the instr_template to a list of values that could be placed there
- `regex`: regular expression that can be used to find identifiers in the instr_template
- `output_file`: `.h` file where C functions to generate the instructions will be written
- `test_cases`: list of tuples (instr_name, arguments, assembly) representing test cases and ground truths. The instr_name and order of arguments should match what is provided in `templates`; `assembly` will be assembled using `asm` to generate the ground truth.

If you implement these in `my_arch_config.py` then you can run
```
python3 derive.py my_arch_config
```

```
$ python3 derive.py -h
usage: derive.py [-h] config

Derive assembly instructions.

positional arguments:
  config      The configuration python file to use (without .py).

optional arguments:
  -h, --help  show this help message and exit
```