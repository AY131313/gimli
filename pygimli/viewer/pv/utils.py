import tempfile
import numpy as np

import pygimli as pg

pv = pg.optImport('pyvista', requiredFor="properly visualize 3D data")


def pgMesh2pvMesh(mesh, data=None, label=None):
    """
    pyGIMLi's mesh format is different from pyvista's needs,
    some preparation is necessary.

    Parameters
    ----------
    mesh: pg.Mesh
        Structure generated by pyGIMLi to display.
    data: iterable
        Parameter to distribute to cells/nodes.
    """
    _, tmp = tempfile.mkstemp(suffix=".vtk")
    # export given mesh temporarily is the easiest and fastest option ATM
    
    mesh.exportVTK(tmp)
    grid = pv.read(tmp)

    # check for parameters inside the pg.Mesh
    for key, values in mesh.dataMap():
        if len(values) == mesh.cellCount():
            grid.cell_data[key] = np.asarray(values)
        elif len(values) == mesh.nodeCount():
            grid.point_data[key] = np.asarray(values)

    for k in grid.cell_data.items():
        if k[0] == 'Marker':
            if len(k[1]) == mesh.boundaryCount():
                grid.rename_array('Marker', 'Boundary marker')
            elif len(k[1]) == mesh.cellCount():
                grid.rename_array('Marker', 'Cell marker')


    # check the given data as well
    try:
        if data is not None:
            if len(data) == mesh.cellCount():
                grid.cell_data[label] = np.asarray(data)
            elif len(data) == mesh.nodeCount():
                grid.point_data[label] = np.asarray(data)
            else:
                pg.warn("Given data fits neither cell count nor node count:")
                pg.warn("{} vs. {} vs. {}".format(len(data), mesh.cellCount(),
                                                mesh.nodeCount()))
    except Exception as e:
        print(e)
        pg.error("fix pyvista bindings")

    # print(grid.cell_data)
    # print(grid.point_data)

    if label is None:
        # last data that was added
        label = grid.array_names[-1]
    elif label not in grid.array_names:
        pg.warn("Given label '{}' was not found.".format(label))
        label = grid.array_names[-1]


    grid.set_active_scalars(label)

    return grid
