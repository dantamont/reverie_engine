import sys


def is_venv():
    """Return whether or not a virtual environment is running"""
    return (hasattr(sys, 'real_prefix') or
            (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix))