#
# This file is part of GreatFET
#

from ..interface import GreatFETInterface

class CC_DEBUG(GreatFETInterface):
    """
    Class representing a GreatFET TODO: name it
    """

    def __init__(self, board):
        self.board = board
        self.api = self.board.apis.swra124

    def initialize(self):
        self.api.setup()
        self.api.debug_init()
        
    def get_chip_id(self):
        return(self.api.get_chip_id())

    def read_status(self):
        return(self.api.read_status())
