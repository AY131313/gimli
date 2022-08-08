#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Module containing submodules for various geophysical methods.
"""

from .constants import Constants

from .ert import ERTManager, ERTModelling, VESManager
from .em import VMDTimeDomainModelling
from .traveltime import TravelTimeManager

Refraction = TravelTimeManager  # Backward compatibility to pg 1.0

from .em import FDEM, TDEM
from .SIP import SIPSpectrum, SpectrumManager
from .sNMR import MRS

constants = Constants

# from . gravimetry import Gravimetry
# from . seismics import *

# __all__ = ["FDEM", "TDEM", "MRS", "SIPSpectrum", "Refraction"]
