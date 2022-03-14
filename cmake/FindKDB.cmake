#[=======================================================================[.rst:
FindKDB
---------

Find KDB include dirs and libraries as shipped by KxSystem at `KxSystems/kdb
<https://github.com/KxSystems/kdb>`_. More information on KDBs interop with 
C/C++ can be found `here <https://code.kx.com/q/interfaces/c-client-for-q/>`_
and `here <https://code.kx.com/q/interfaces/using-c-functions/>`_. The latter
link will inform you that any target you link to from this find module should
be a shared object 

The following components are supported:

* ``NOSSL``: KDB object without SSL support.
* ``SSL``: KDB object with SSL support (requires OpenSSL::SSL).

If no ``COMPONENTS`` are specified, ``SSL`` is preferred but falls 
back to ``NOSSL``.

Use this module by invoking :command:`find_package` with the form:

.. code-block:: cmake

  find_package(KDB
    [REQUIRED]             # Fail with error if KDB is not found
    [COMPONENTS <libs>...] # KDB libraries by their name "NOSSL" or "SSL"
    [OPTIONAL_COMPONENTS <libs>...]
                           # Optional KDB libraries by their name
    )                      # e.g. "NOSSL" or "SSL"

This module finds headers and requested component objects as provided by KxSystems.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``KDB_FOUND``
  True if headers and requested components were found.

``KDB_INCLUDE_DIRS``
  KDB include directories.

``KDB_OBJS``
  SSL object if available, else NO SSL object. Only one should be linked.

``KDB_<COMPONENT>_FOUND``
  True if component ``<COMPONENT>`` was found (``<COMPONENT>`` name is upper-case).

``KDB_<COMPONENT>_OBJ``
  Objects to link for component ``<COMPONENT>``.

Cache variables
^^^^^^^^^^^^^^^

Search results are saved persistently in CMake cache entries:

``KDB_INCLUDE_DIR``
  Directory containing KDB headers.

``KDB_OBJ``
  KDB without SSL object.

``KDB_SSL_OBJ``
  KDB with SSL object.

Hints
^^^^^

This module reads hints about search locations from variables:

``KDB_ROOT``
  Preferred installation prefix.

``KDB_INCLUDE_DIR_HINT``
  Preferred include directory e.g. ``<prefix>/include``.

``KDB_OBJ_DIR_HINT``
  Preferred object file directory e.g. ``<prefix>/obj``.

Users may set these hints or results as ``CACHE`` entries.  Projects
should not read these entries directly but instead use the above
result variables.  One may specify these as environment variables if 
they are not specified as CMake variables or cache entries.

This module first searches for the KDB header files using the above
hint variables (excluding ``KDB_OBJ_DIR_HINT``) and saves the result in
``KDB_INCLUDE_DIR``.  Then it searches for requested component objects
using the above hints (excluding ``KDB_INCLUDE_DIR_HINT``). It saves the
individual objects locations in ``KDB_[SSL_]OBJ``.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``KDB::NOSSL``
  Target for KDB library without SSL support

``KDB::SSL``
  Target for KDB library with SSL support

``KDB::KDB``
  Alias for KDB::SSL if SSL found, else KDB::NOSSL.  

Implicit third-party dependencies, such as ``OpenSSL::SSL``, will be 
automatically detected and satisfied.

It is important to note that the imported targets behave differently
than variables created by this module: multiple calls to
:command:`find_package(KDB)` in the same directory or sub-directories with
different options (e.g. static or shared) will not override the
values of the targets created by the first call.

Examples
^^^^^^^^

Find and KDB:

.. code-block:: cmake

  find_package(KDB REQUIRED)
  target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> KDB::KDB)

Find KDB preferring SSL, but settle for NOSSL (effectively the same as the above)

.. code-block:: cmake

  find_package(KDB REQUIRED COMPONENTS NOSSL OPTIONAL_COMPONENTS SSL)
  target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> KDB::KDB)

Find KDB with SSL (Requires OpenSSL::SSL to be found)

.. code-block:: cmake

  find_package(KDB REQUIRED COMPONENTS SSL)
  target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> KDB::KDB) # or KDB::SSL

#]=======================================================================]

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.0)
    message(FATAL_ERROR "Find Module requires CMake 3.0+ as it is target orientated.")
endif()

set(ARM_HINT "")
IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
    set(ARM_HINT "arm")    
ENDIF()

set(BIT_HINT 64)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(BIT_HINT 32)
endif()

if (WIN32)
    message(FATAL_ERRO "Windows not currently supported by this find module")
    set(OS_HINT "w")
elseif(APPLE)
    set(OS_HINT "m")
elseif(UNIX)
    set(OS_HINT "l")
else()
    message(FATAL_ERROR "Platform unknown. Not Windows, Mac, or Unix.")
endif()

set(ARCH_HINT "${OS_HINT}${BIT_HINT}${ARM_HINT}")


# Look for the kx directory which includes k.h and kx.h
find_path(KDB_INCLUDE_DIR kx HINTS ${KDB_INCLUDE_DIR_HINT} ${KDB_ROOT} PATH_SUFFIXES "include")

# Search for component objects
find_file(KDB_OBJ c.o HINTS ${KDB_OBJ_DIR_HINT} ${KDB_ROOT} PATH_SUFFIXES "obj/${ARCH_HINT}" "obj" "${ARCH_HINT}")
find_file(KDB_SSL_OBJ e.o HINTS ${KDB_OBJ_DIR_HINT} ${KDB_ROOT} PATH_SUFFIXES "obj/${ARCH_HINT}" "obj" "${ARCH_HINT}")

# Setup no ssl target variables
if (KDB_INCLUDE_DIR AND KDB_OBJ)
    set(KDB_NOSSL_FOUND TRUE)
    mark_as_advanced(KDB_INCLUDE_DIR KDB_OBJ)
    set(KDB_INCLUDE_DIRS ${KDB_INCLUDE_DIR})
    set(KDB_NOSSL_OBJ ${KDB_OBJ})
endif()

# Setup ssl target variables (if openssl found). Quiet as not required.
find_package(OpenSSL QUIET COMPONENTS SSL)
if (KDB_INCLUDE_DIR AND KDB_SSL_OBJ AND OPENSSL_SSL_FOUND)
    set(KDB_SSL_FOUND TRUE)
    mark_as_advanced(KDB_INCLUDE_DIR KDB_SSL_OBJ)
    set(KDB_INCLUDE_DIRS ${KDB_INCLUDE_DIR})
    set(KDB_SSL_OBJ ${KDB_SSL_OBJ})
endif()

# Set kdb target objects (prefer ssl)
if (KDB_SSL_OBJ)
    set(KDB_OBJS ${KDB_SSL_OBJ})
elseif(KDB_OBJ)
    set(KDB_OBJS ${KDB_OBJ})
endif()

# Check optional and required components are found, if so set KDB_FOUND to TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDB HANDLE_COMPONENTS)

# Make targets
if (KDB_NOSSL_FOUND AND NOT TARGET KDB::NOSSL)
    add_library(KDB::NOSSL OBJECT IMPORTED)
    set_target_properties(KDB::NOSSL PROPERTIES
        IMPORTED_OBJECTS ${KDB_NOSSL_OBJ}
        INTERFACE_INCLUDE_DIRECTORIES ${KDB_INCLUDE_DIRS})

    # TODO: Make targets to allow KXVER2 and KXVER3
    target_compile_definitions(KDB::NOSSL INTERFACE KXVER=3)
endif()

if (KDB_SSL_FOUND AND NOT TARGET KDB::SSL)
    add_library(KDB::SSL OBJECT IMPORTED)
    set_target_properties(KDB::SSL PROPERTIES
        IMPORTED_OBJECTS ${KDB_SSL_OBJ}
        INTERFACE_INCLUDE_DIRECTORIES ${KDB_INCLUDE_DIRS})

    target_link_libraries(KDB::SSL INTERFACE OpenSSL::SSL)

    # TODO: Make targets to allow KXVER2 and KXVER3
    target_compile_definitions(KDB::SSL INTERFACE KXVER=3)
endif()

if (NOT TARGET KDB::KDB)
    if (KDB_SSL_FOUND)
        add_library(KDB::KDB ALIAS KDB::SSL)
    else()
        add_library(KDB::KDB ALIAS KDB::NOSSL)
    endif()
endif()
