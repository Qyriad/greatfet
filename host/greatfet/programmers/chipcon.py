#
# This file is part of GreatFET
#

from ..programmer import GreatFETProgrammer


def create_programmer(board, *args, **kwargs):
    """ Creates a representative programmer for this module. """

    return ChipconProgrammer(board, *args, **kwargs)


class ChipconProgrammer(GreatFETProgrammer):
    """
    Class representing a GreatFET TODO: name it
    """


    def _split_linear_address(self, linear_address):
        """ Takes a linear address and returns a bank index, and the page address as separate bytes.

        Paramters:
            linear_address -- The address to convert.

            Returns -- A tuple of the three values: (bank, page_address_high, page_address_low)
        """

        # Each bank is a 15-bit address space.
        # The bank itself is 8 bits.
        bank = linear_address >> 15

        if bank > 0xFF:
            raise ValueError("Linear address is too big: must fit in 8 bits!")

        page_address = linear_address & 0x7FFF

        page_address_high = page_address >> 8
        page_address_low = page_address & 0x00FF

        return (bank, page_address_high, page_address_low)


    def __init__(self, board):
        self.board = board
        self.api = self.board.apis.swra124

        self.api.setup()


    def debug_init(self):
        self.api.debug_init()


    def get_chip_id(self):
        return(self.api.get_chip_id())


    def read_status(self):
        return(self.api.read_status())


    def run_instruction(self, *instruction):
        """ Executes a single instruction on the target without
        incrementing the program counter.

        Paramters:
            instruction -- A bytes instance or a list of integers representing the opcodes to execute.
        """

        if not isinstance(instruction[0], bytes):
            instruction = bytes(instruction)
        else:
            instruction = instruction[0]

        return self.api.debug_instr(instruction)


    def read_code_memory(self, linear_address, length):
        """ Reads a section of code memory.

        Paramters:
            linear_address -- The address in code memory to read from. It will be converted into an 8-bit bank
                index and a 15-bit address within that bank.
            length -- The amount of data to read.
        """

        # Assembly opcodes used as recommended in SWRA124.

        output = bytearray()

        # Each bank is a 15-bit address space.
        # The bank index itself is 8 bits.
        bank = linear_address >> 15

        if bank > 0xFF:
            raise ValueError("Linear address is too big: (bank * 16) must fit in a byte")

        page_address = linear_address & 0x7FFF

        page_address_high = page_address >> 8
        page_address_low  = page_address & 0xFF

        self.run_instruction(0x75, 0xc7, (bank * 16) + 1)               # MOV MEMCTR, (bank * 16) + 1
        self.run_instruction(0x90, page_address_high, page_address_low) # MOV DPTR, address

        for n in range(length):
            self.run_instruction(0xE4)                # CLR A
            output.append(self.run_instruction(0x93)) # MOVC A, @A+DPTR
            self.run_instruction(0xA3)                # INC DPTR

        return output


    def read_xdata_memory(self, linear_address, length):
        """ Reads a section of xdata memory.

        Paramters:
            linear_address -- The address in code memory to read from. It will be converted into an 8-bit bank
                index and a 15-bit address within that bank.
            length -- The amount of data to read.
        """

        # Assembly opcodes used as recommended in SWRA124.

        output = bytearray()

        bank, page_address_low, page_address_high = self._split_linear_address(linear_address)

        self.run_instruction(0x90, page_address_high, page_address_low) # MOV DPTR, address

        for n in range(length):
            output.append(self.run_instruction(0xE0)) # MOVX A, @DPTR; (output[n] = A)
            self.run_instruction(0xA3) # INC DPTR

        return output
