import abc
import subprocess
import os
import sys

class Assembler(abc.ABC):
    """Abstract base class representing an assembler."""

    @abc.abstractmethod
    def __init__(self, config_dict):
        """Initialize an Assembler (should not be called directly).

        Args:
            config_dict (dict): Configuration options.

        Raises:
            NotImplementedError: Assembler is an abstract class
        """
        raise NotImplementedError("Assembler is an abstract class.")

    @abc.abstractmethod
    def assemble(self, filename):
        """Assembles a given file and returns the resulting binary.

        Args:
            filename (str): The name of the file to assemble.

        Returns:
            bytes: The assembled binary.
        """
        
        raise NotImplementedError("Subclasses must implement Assembler.assemble!")

    @abc.abstractmethod
    def cleanup(self):
        """Cleans up any temporary files created during the assembly process."""
        pass

    @abc.abstractmethod
    def arch(self):
        """Returns a string denoting the assembler architecture."""
        raise NotImplementedError("Subclasses must implement Assembler.arch!")

class ArmNoneEabiAssembler(Assembler):
    """arm-none-eabi assembler (for r/pi)"""

    memmap = None
    asm_file = None
    obj_file = None
    bin_file = None


    def __init__(self, config_dict):
        """Sets up an instance of an ArmNoneEabiAssembler.
        
        Args:
            config_dict: A dictionary containing the assembler configuration.
        """

        self.memmap = config_dict["memmap"]
        self.asm_file = config_dict["filepath"] + ".S"
        self.obj_file = config_dict["filepath"] + ".o"
        self.bin_file = config_dict["filepath"] + ".bin"


    def assemble(self, instruction):
        """Assembles a given file and returns the resulting binary.

        Args:
            filename (str): The name of the file to assemble.

        Returns:
            bytes: The assembled binary.
        """

        # Create temporary assembly file
        with open(self.asm_file, "w+") as f:
            f.write(instruction + "\n")

        
        # Assemble the file
        retcode = subprocess.call(["arm-none-eabi-gcc", "-c", self.asm_file, "-o", self.obj_file])
        if retcode != 0:
            raise RuntimeError(f"Error assembling instruction: `{instruction}`")

        # Link the object file into a binary
        retcode = subprocess.call(["arm-none-eabi-objcopy", self.obj_file, "-O", "binary", self.bin_file])
        if retcode != 0:
            raise RuntimeError(f"Error objcopying: `{self.obj_file}`")

        # Read the binary
        with open(self.bin_file, "rb") as f:
            binary = f.read()

        if (len(binary) != 4):
            raise RuntimeError(f"Expected 4 byte instruction but got {len(binary)} bytes!")

        # convert from little endian
        return bytes([binary[3], binary[2], binary[1], binary[0]])


    def cleanup(self):
        """Cleans up any temporary files created during the assembly process."""

        os.remove(self.asm_file)
        os.remove(self.obj_file)
        os.remove(self.bin_file)

    def arch(self):
        """Returns a string denoting the assembler architecture."""
        return "arm"