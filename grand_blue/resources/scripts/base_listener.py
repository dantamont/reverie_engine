"""
Script for an examplar event listener implementation
"""

from PythonQt.events import CustomEvent

class BaseListener(object):
    """
    An example event listener
    """

    @property
    def input(self):
        """Return input handler"""
        return self.engine.input_handler()

    @property
    def engine(self):
        """Return engine from associated scene object"""
        return self.scene_object.engine()

    @property
    def resources(self):
        """Return resource cache from the core engine"""
        return self.engine.resource_cache()

    @property
    def transform(self):
        """Return engine from associated scene object"""
        return self.scene_object.transform()

    @property
    def scene(self):
        """Return scene from associated scene object"""
        return self.scene_object.scene()

    def __init__(self, scene_object):
        """Create the listener"""
        self.scene_object = scene_object
        return

    def event_test(self, event: CustomEvent):
        """Test to decide whether or not to perform 
        the listener action
        """
        return True

    def perform(self, event: CustomEvent):
        """Perform the action for this listener"""
        if DEBUG_MODE:
            self.log_info("Performing listener action")
        return 1

    def log_info(self, info: str=""):
        """ Log given string as info """
        if DEBUG_MODE:
            print(info)
            return 1
        return 0