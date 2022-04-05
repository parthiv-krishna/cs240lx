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
