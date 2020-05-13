"""
Script for an examplar event listener implementation
"""

class TestListener(BaseListener):
    """
    An example event listener
    """

    def __init__(self, scene_object):
        """Create the listener"""
        super().__init__(scene_object)
        return

    def event_test(self, event: CustomEvent):
        """Test to decide whether or not to perform 
        the listener action
        """
        return True

    def perform(self, event: CustomEvent):
        """Perform the action for this listener"""
        # TODO: take an actual event
        self.log_info("Performing listener action")
        self.log_info(event.dataMap())
        return 1
