# -*- coding: utf-8 -*-
"""pyGIMLi - Inversion Frameworks.

These are basic inversion frameworks that usually need a forward operator to run.
"""
import numpy as np

import pygimli as pg


class Inversion(object):
    """Basic inversion framework.

    Attributes
    ----------
    verbose : bool
        Give verbose output
    debug : bool
        Give debug output
    """
    def __init__(self, fop=None, inv=None, **kwargs):
        self._verbose = kwargs.pop('verbose', False)
        self._debug = kwargs.pop('debug', False)

        # If this class or its derived is a Framework the _inv holds another
        # Inversion which allows us ........
        # this will be probably removed in the future
        self.isFrameWork = False

        self._dataVals = None
        self._errorVals = None

        # might be overwritten
        self.transData = pg.RTransLin()

        self._inv = None

        if inv is not None:
            self._inv = inv
            self.isFrameWork = True
        else:
            self._inv = pg.Inversion(self._verbose, self._debug)

        self.axs = None # for showProgress only

        self.maxIter = kwargs.pop('maxIter', 20)

        if fop is not None:
            self.setForwardOperator(fop)

        #self.inv.setDeltaPhiAbortPercent(0.5)

    @property
    def inv(self):
        if self.isFrameWork:
            return self._inv.inv
        else:
            return self._inv

    @property
    def verbose(self):
        return self._verbose
    @verbose.setter
    def verbose(self, v):
        self._verbose = v
        if self.inv is not None:
            self.inv.setVerbose(self._verbose)

    @property
    def debug(self):
        return self._debug
    @debug.setter
    def debug(self, v):
        self._debug = v
        if self.inv is not None:
            self.inv.setDosave(self._debug)

    @property
    def maxIter(self):
        return self.inv.maxIter()
    @maxIter.setter
    def maxIter(self, v):
        if self.inv is not None:
            self.inv.setMaxIter(v)

    @property
    def response(self):
        if len(self.inv.response()) > 0:
            return self.inv.response()
        else:
            raise Exception("There was no inversion run so there is no response yet")
    @property
    def model(self):
        if len(self.inv.model()) > 0:
            return self.inv.model()
        else:
            raise Exception("There was no inversion run so there is last model")

    # backward compatibility
    @property
    def dataErrs(self):
        return self._errorVals
    @dataErrs.setter
    def dataErrs(self, v):
        self._errorVals = v

    @property
    def parameterCount(self):
        return self.fop.regionManager().parameterCount()

    def echoStatus(self):
        self.inv.echoStatus()

    def setDeltaChiStop(self, it):
        self.inv.setDeltaPhiAbortPercent(it)

    def setForwardOperator(self, fop):
        self.fop = fop
        self._inv.setForwardOperator(fop)

    def setData(self, data):
        QUESTION_ISNEEDED
        if isinstance(data, pg.DataContainer):
            raise Exception("should not been here .. its Managers job")
            self.fop.setData(data)
        else:
            self._dataVals = data

    def run(self, dataVals, errorVals, **kwargs):
        """Run inversion.

        The inversion will always start from the starting model given to the
        forward operator.
        If you want to run the inversion from a specified prior model,
        e.g., from a other run, set this model as starting model to the FOP
        (fop.setStartModel).
        Any self.inv.setModel() settings will be overwritten.
        """
        if self.isFrameWork:
            return self._inv.run(dataVals, errorVals, **kwargs)

        if self.fop is None:
            raise Exception("Need a valid forward operator for inversion run.")

        self.verbose = kwargs.pop('verbose', self.verbose)
        self.maxIter = kwargs.pop('maxIter', self.maxIter)


        startModel = kwargs.pop('startModel', None)
        if type(startModel) is float or type(startModel) is int:
            nModel = self.parameterCount
            startModel = pg.Vector(nModel, startModel)
            self.fop.setStartModel(startModel)
        elif hasattr(startModel, '__iter__'):
            self.fop.setStartModel(startModel)


        showProgress = kwargs.pop('showProgress', False)

        lam = kwargs.pop('lam', 20)

        self.inv.setTransModel(self.fop.transModel)
        self.inv.setTransData(self.transData)

        if dataVals is not None:
            self._dataVals = dataVals

        if errorVals is not None:
            self._errorVals = errorVals

        if self._dataVals is None:
            raise Exception("Inversion framework need data values to run")

        if self._errorVals is None:
            raise Exception("Inversion framework need data error values to run")


        self.inv.setData(self._dataVals)
        self.inv.setRelativeError(self._errorVals)
        self.inv.setLambda(lam)

        maxIter = self.maxIter
        self.maxIter = 1

        if self.verbose:
            print("inv.start()")

        ### To ensure reproducability of the run call inv.start() will
        ### reset self.inv.model() to fop.startModel().
        self.inv.start()
        self.maxIter = maxIter

        if showProgress:
            self.showProgress(showProgress)

        lastChi2 = self.inv.chi2()
        chi2History = [lastChi2]

        for i in range(1, self.maxIter):
            if self.verbose:
                print("inv.iter", i, "...", end='')

            self.inv.oneStep()
            resp = self.inv.response()
            chi2 = self.inv.chi2()

            if showProgress:
                self.showProgress(showProgress)


            self.inv.setLambda(self.inv.getLambda() * self.inv.lambdaFactor())

            if self.inv.robustData():
                self.inv.robustWeighting()

            if self.inv.blockyModel():
                self.inv.constrainBlocky()

            chi2History.append(chi2)

            if self.verbose:
                print("chi² =", round(chi2,2),
                      '(dchi² =', round((1.-lastChi2/chi2) * 100, 2), "%), lam:", self.inv.getLambda())

            if chi2 < 1:
                if self.verbose:
                    print("Abbort criteria reached: chi² < 1")
                break

            if abs((1-lastChi2/chi2) * 100) < self.inv.deltaPhiAbortPercent():
                if self.verbose:
                    print(lastChi2/chi2, "Abbort criteria reached: dChi²=",
                          round((1-lastChi2/chi2) * 100, 2),
                         "(", self.inv.deltaPhiAbortPercent(), '%)')
                break

            lastChi2 = chi2

        if len(kwargs.keys()) > 0:
            print("Warning! unhandled keyword arguments", kwargs)

        return self.inv.model()

    def showProgress(self, style='all'):
        """Called if showProgress=True is set for the inversion run."""

        if self.axs is None:
            axs = None
            if style == 'all' or style == True:
                fig, axs = pg.plt.subplots(1, 2)
            elif style == 'Model':
                fig, axs = pg.plt.subplots(1, 1)
            self.axs = axs

        ax = self.axs

        if style == 'Model':
            ax.clear()
            self.fop.drawModel(ax, self.inv.model())
        else:
            ax[0].clear()
            self.fop.drawModel(ax[0], self.inv.model())

            ax[1].clear()
            self.fop.drawData(ax[1], self._dataVals, self._errorVals, label='Data')
            self.fop.drawData(ax[1], self.inv.response(), label='Response')

            ax[1].text(0.01, 0.96,
                    "iter: %d, rrms: %.2g, $\chi^2$: %.2g" %
                        (self.inv.iter(), self.inv.relrms(), self.inv.chi2()),
                        transform=ax[1].transAxes)

        pg.plt.pause(0.05)


class MarquardtInversion(Inversion):
    """Marquardt scheme (local damping with decreasing regularization strength
    """
    def __init__(self, fop=None, **kwargs):
        super(MarquardtInversion, self).__init__(fop, **kwargs)
        self.inv.setLocalRegularization(True)
        self.inv.stopAtChi1(False)
        self.inv.setLambdaFactor(0.9)

    def run(self, data, error, **kwargs):

        self.fop.regionManager().setConstraintType(0)

        return super(MarquardtInversion, self).run(data, error, **kwargs)


class Block1DInversion(MarquardtInversion):
    def __init__(self, fop=None, **kwargs):
        super(Block1DInversion, self).__init__(fop, **kwargs)

    def run(self, dataVals, errVals, nLayer=4, **kwargs):

        #if len(self.fop.startModel()) == 0:
        # somehow update model space if nlayers has been changed
        self.fop.createStartModel(dataVals, nLayer)

        return super(Block1DInversion, self).run(dataVals, errVals, **kwargs)


class MeshInversion(Inversion):
    def __init__(self, fop=None, **kwargs):
        super(MeshInversion, self).__init__(fop, **kwargs)
        self._model = None

    def setMesh(self, mesh):
        self.fop.setMesh(mesh)

    @property
    def model(self):
        return self._model
    @model.setter
    def model(self, m):
        self._model = m

    def run(self, dataVals, errVals, mesh=None, zWeight=1.0, **kwargs):

        print("------------------------------------------------")
        print("MeshInversion:run()")
        if mesh is not None:
            self.setMesh(mesh)

        # check for valid mesh here.

        # configure regions defaults here??

        self.fop.regionManager().setZWeight(zWeight)

        model = super(MeshInversion, self).run(dataVals, errVals, **kwargs)

        self.model = model(self.fop.regionManager().paraDomain().cellMarkers())
        return self.model


class PetroInversion(Inversion):
    def __init__(self, petro, mgr=None, fop=None, **kwargs):
        self.mgr = None

        if mgr is not None:
            fop = self.mgr.createForwardOperator(**kwargs)

        f = None
        if fop is not None:
            f = pg.frameworks.PetroModelling(fop, petro)

        super(PetroInversion, self).__init__(f, **kwargs)


    def run(self, dataVals, errVals, **kwargs):
        """
        """
        ## this is Managers job
        #if isinstance(data, pg.DataContainer):
            #self.fop.setDataContainer(data)
            #dataVals = self.mgr.dataVals(data)
            #errVals = self.mgr.relErrVals(data)
        #else:
            #raise Exception("Implement me")
        print("------------------------------------------------")
        print("PetroInversion:run()")

        if 'limits' in kwargs:
            limits = kwargs.pop('limits', [0., 1.])
            self.fop._transModel.setLowerBound(limits[0])
            self.fop._transModel.setUpperBound(limits[1])

        #self.tM.setLowerBound(limits[0])
        #self.tM.setUpperBound(limits[1])
        #self.inv.setTransModel(self.tM)


        #**kwargs['startModel'] automatic here


        return super(PetroInversion, self).run(dataVals, errVals, **kwargs)































class MeshInversion0(Inversion):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        INUSEQUESTION

    def setMesh(self, mesh):
        self.fop.setMesh(mesh)

    def invert(self, data=None, mesh=None, lam=20, **kwargs):
        INUSEQUESTION
        if data is not None:
            self.setData(data)

        if mesh is not None:
            self.setMesh(mesh)

        startModel = kwargs.pop('startModel', None)
        nModel = self.fop.regionManager().parameterCount()
        startModel = pg.Vector(nModel, startModel)
        self.fop.setStartModel(startModel)

        zWeight = kwargs.pop('zWeight', 1.0)
        self.fop.regionManager().setZWeight(zWeight)
        self.inv.setData(self.dataVals)
        self.inv.setRelativeError(self.errorVals)
        self.inv.setLambda(lam)

        self.mod = self.inv.run()
        self.mod = self.mod(self.fop.regionManager().paraDomain().cellMarkers())
        return self.mod