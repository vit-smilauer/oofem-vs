# Much to organize
set (core_unsorted
    classfactory.C
    femcmpnn.C domain.C timestep.C metastep.C gausspoint.C
    cltypes.C timer.C dictionary.C heap.C grid.C
    connectivitytable.C error.C mathfem.C logger.C util.C
    initmodulemanager.C initmodule.C initialcondition.C
    assemblercallback.C
    homogenize.C
    nonlocalbarrier.C
    geotoolbox.C geometry.C
    datastream.C
    set.C
    weakperiodicbc.C
    solutionbasedshapefunction.C
    fracturemanager.C
    neumannmomentload.C
    parallelcontext.C
    contact/contactmanager.C
    contact/contactdefinition.C
    contact/contactelement.C
    prescribedmean.C
    bctracker.C
    # Semi sorted:
    errorestimator.C meshqualityerrorestimator.C remeshingcrit.C
    sloangraph.C sloangraphnode.C sloanlevelstruct.C
    eleminterpunknownmapper.C primaryunknownmapper.C materialmappingalgorithm.C
    nonlocalmaterialext.C randommaterialext.C
    inputrecord.C oofemtxtinputrecord.C dynamicinputrecord.C
    dynamicdatareader.C oofemtxtdatareader.C tokenizer.C parser.C
    spatiallocalizer.C dummylocalizer.C octreelocalizer.C
    integrationrule.C gaussintegrationrule.C lobattoir.C
    smoothednodalintvarfield.C dofmanvalfield.C
    # Deprecated?
    rowcol.C skyline.C skylineu.C
    ldltfact.C
    inverseit.C subspaceit.C gjacobi.C
    #
    symcompcol.C compcol.C
    unstructuredgridfield.C
    # 
    loadbalancer.C
    #
    vtkhdf5reader.C
    #
    oofemcfg.C
    )

#oofegutils

if (USE_SPOOLES)
    list (APPEND core_unsorted spoolessparsemtrx.C spoolessolver.C)
endif ()

if (USE_IML)
    list (APPEND core_unsorted
        iml/dyncomprow.C iml/dyncompcol.C
        iml/precond.C iml/voidprecond.C iml/icprecond.C iml/iluprecond.C iml/ilucomprowprecond.C iml/diagpre.C
        iml/imlsolver.C
        )
endif ()

if (USE_T3D)
    list (APPEND core_unsorted
        T3d/src/t3d.c T3d/src/option.c T3d/src/error.c T3d/src/list.c T3d/src/draw.c T3d/src/draw1.c T3d/src/draw2.c T3d/src/drawpc.c
		T3d/src/vertex.c T3d/src/vertex1.c T3d/src/region.c T3d/src/patch.c T3d/src/shell.c T3d/src/bernstein.c T3d/src/ctrlpoint.c
		T3d/src/curve.c T3d/src/curve1.c T3d/src/curve2.c T3d/src/curve3.c 
		T3d/src/surface.c T3d/src/surface1.c T3d/src/surface2.c T3d/src/surface3.c 
		T3d/src/input.c T3d/src/input1.c T3d/src/input2.c T3d/src/input3.c T3d/src/input4.c T3d/src/input5.c T3d/src/input6.c 
		T3d/src/octree.c T3d/src/octree1.c T3d/src/octree2.c T3d/src/domain.c T3d/src/element.c 
		T3d/src/discret.c T3d/src/discret1.c T3d/src/discret2.c T3d/src/discret3.c 
		T3d/src/discret4.c T3d/src/discret5.c T3d/src/discret6.c 
		T3d/src/smooth.c T3d/src/smooth1.c T3d/src/quality.c T3d/src/renum.c T3d/src/output.c T3d/src/output1.c
        )
endif ()

if (USE_SLEPC)
    list (APPEND core_unsorted slepcsolver.C)
endif ()

if (USE_SUPERLU_MT)
  list (APPEND core_unsorted superlusolver.C)
endif ()


set (core_oofeg oofeggraphiccontext.C)

set (core_nodalrecovery
    nodalrecoverymodel.C
    zznodalrecoverymodel.C
    nodalaveragingrecoverymodel.C
    sprnodalrecoverymodel.C
    )

set (core_dofs
    dof.C
    masterdof.C
    simpleslavedof.C
    slavedof.C
    activedof.C
    )

set (core_nm
    sparselinsystemnm.C
    sparsenonlinsystemnm.C
    nrsolver.C
    dynamicrelaxationsolver.C
    linesearch.C
    calmls.C
    staggeredsolver.C
    )

if (USE_PETSC)
    list (APPEND core_nm petscsolver.C petscsparsemtrx.C)
endif ()

if (USE_INTEL_MKL_PARDISO OR USE_ONEAPI)
    message (STATUS "    MKL Solver is enabled for compilation")
    list (APPEND core_nm mklpardisosolver.C)
endif ()

if (USE_PARDISO_ORG)
  list (APPEND core_nm pardisoprojectorgsolver.C)
endif ()


set (core_fields
    fieldmanager.C
    field.C
    primaryfield.C
    intvarfield.C
    maskedprimaryfield.C
    dofdistributedprimaryfield.C
    eigenvectorprimaryfield.C
    uniformgridfield.C
    )

set (core_ltf
    function.C
    scalarfunction.C
    constantfunction.C
    heavisidetimefunction.C
    peakfunction.C
    piecewiselinfunction.C
    periodicpiecewiselinfunction.C
    calculatorfunction.C
    localgaussianrandomfunction.C
    interpolatingfunction.C
    )

set (core_cs
    crosssection.C
    emptycs.C
    dummycrosssection.C
    )

set (core_td
    )

set (core_dofman
    dofmanager.C
    node.C
    slavenode.C
    generalslavenode.C
    hangingnode.C
    qcnode.C
    rigidarmnode.C
    elementinternaldofman.C
    elementside.C
    latticedirichletcouplingnode.C
    latticeneumanncouplingnode.C
    )

set (core_material
    material.C
    dummymaterial.C
    )

set (core_export
    outputmanager.C
    exportmodule.C
    exportmodulemanager.C
    exportregion.C
    outputexportmodule.C
    errorcheckingexportmodule.C
    vtkexportmodule.C
    vtkbaseexportmodule.C
    vtkxmlexportmodule.C
    vtkmemoryexportmodule.C
    vtkxmlperiodicexportmodule.C
    vtkxmllatticeexportmodule.C
    vtkpfemexportmodule.C
    vtkxfemexportmodule.C
    homexportmodule.C
    matlabexportmodule.C
    gpexportmodule.C
    solutionstatusexportmodule.C
    vtkhdf5exportmodule.C
    )

set (core_monitors
    monitormanager.C
    monitor.C
    )

set (core_iga
    iga/iga.C
    iga/feibspline.C
    iga/feinurbs.C
    iga/feitspline.C
    )

set (core_arrays
    intarray.C
    floatarray.C
    floatmatrix.C
    )

set (core_engng
    engngm.C
    staggeredproblem.C
    dummyengngm.C
    )

set (core_element
    element.C
    )

set (core_fei
    feinterpol.C
    feinterpol1d.C
    feinterpol2d.C
    feinterpol3d.C
    fei1dlin.C
    fei1dquad.C
    fei1dhermite.C
    fei2dtrlin.C
    fei2dtrquad.C
    fei2dlinehermite.C
    fei2dlinelin.C
    fei2dlinequad.C
    fei2dquadconst.C
    fei2dquadlin.C
    fei2dquadquad.C
    fei2dquadbiquad.C
    fei3dlinelin.C
    fei3dtrlin.C
    fei3dquadlin.C
    fei3dtrquad.C
    fei3dtetlin.C
    fei3dtetquad.C
    fei3dhexalin.C
    fei3dhexaquad.C
    fei3dhexatriquad.C
    fei3dwedgelin.C 
    fei3dwedgequad.C
    fei3dhexaconst.C
    )

set (core_xfem
    xfem/delaunay.C
    xfem/xfemelementinterface.C
    xfem/enrichmentfunction.C
    xfem/xfemmanager.C
    xfem/enrichmentitem.C
    xfem/geometrybasedei.C
    xfem/hybridei.C
    xfem/directei.C
    xfem/listbasedei.C
    xfem/inclusion.C
    xfem/patchintegrationrule.C
    xfem/propagationlaw.C
    xfem/XFEMDebugTools.C
    xfem/enrichmentfronts/enrichmentfront.C
    xfem/enrichmentfronts/enrichmentfrontdonothing.C
    xfem/enrichmentfronts/enrichmentfrontextend.C
    xfem/enrichmentfronts/enrichmentfrontreducefront.C
    xfem/enrichmentfronts/enrichmentfrontlinbranchfuncradius.C
    xfem/enrichmentfronts/enrichmentfrontlinbranchfunconeel.C
    xfem/enrichmentfronts/enrichmentfrontcohesivebranchfunconeel.C
    xfem/enrichmentfronts/enrichmentfrontintersection.C
    xfem/integrationrules/discsegintegrationrule.C
    xfem/xfemtolerances.C
    xfem/nucleationcriterion.C
    )

set (core_parallel
    communicator.C
    processcomm.C
    problemcomm.C
    combuff.C
    domaintransactionmanager.C
    parallelordering.C
    dyncombuff.C
    wallclockloadbalancermonitor.C
    nonlocalmatwtp.C
    )

if (USE_PARMETIS)
    list (APPEND core_parallel parmetisloadbalancer.C)
endif ()

set (core_bc
    generalboundarycondition.C
    boundarycondition.C
    prescribedgradienthomogenization.C
    prescribedgradient.C
    prescribedgradientbcneumann.C
    prescribedgradientbcweak.C
    prescribedgradientbcweakperiodic.C
    prescribedgradientbcweakdirichlet.C
    prescribedgradientbcperiodic.C
    mixedgradientpressurebc.C
    mixedgradientpressuredirichlet.C
    mixedgradientpressureneumann.C
    mixedgradientpressureweakperiodic.C
    load.C
    boundaryload.C
    pointload.C
    linearedgeload.C
    constantedgeload.C
    constantsurfaceload.C
    nodalload.C
    deadweight.C
    reinforcement.C
    surfacetensionbc.C
    linearconstraintbc.C
    )

if (USE_PYTHON_EXTENSION)
    list (APPEND core_bc userdefdirichletbc.C)
    list (APPEND core_unsorted pythonexpression.C)
endif ()

if (USE_PYBIND_BINDINGS)
    list (APPEND core_unsorted pythonfield.C)
endif ()



set (core_mapping
    mmaclosestiptransfer.C
    mmashapefunctprojection.C
    mmaleastsquareprojection.C
    mmacontainingelementprojection.C
    matstatmapperint.C
    )

set (core_mesher
    t3dinterface.C
    targe2interface.C
    freeminterface.C
    subdivision.C
    trianglemesherinterface.C
    )

set (core
    ${core_unsorted}
    ${core_nodalrecovery}
    ${core_dofs}
    ${core_nm}
    ${core_td}
    ${core_fields}
    ${core_dofman}
    ${core_material}
    ${core_export}
    ${core_monitors}
    ${core_ltf}
    ${core_cs}
    ${core_arrays}
    ${core_engng}
    ${core_element}
    ${core_fei}
    ${core_xfem}
    ${core_bc}
    ${core_iga}
    ${core_mesher}
    ${core_mapping}
    )

if (USE_MPI_PARALLEL)
    list (APPEND core ${core_parallel})
endif ()

if (USE_OOFEG)
    list (APPEND core ${core_oofeg})
endif ()


add_library (core
    ${LIB_TYPE}
    ${core}
    )
