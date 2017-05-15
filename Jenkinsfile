/*
 * h5cc_test Jenkinsfile
 */

node ("qt && boost && root && fedora") {

    dir("code") {
        stage("Checkout projects") {
            checkout([
                $class: 'GitSCM',
                extensions: [[
                    $class: 'SubmoduleOption',
                    disableSubmodules: false,
                    parentCredentials: false,
                    recursiveSubmodules: true,
                    reference: '',
                    trackingSubmodules: false
                ]],
                submoduleCfg: []
            ])
        }
    }

    dir("build") {
        stage("Run CMake") {
            sh 'rm -rf ./*'
            sh '/opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake --version'
            sh "source /opt/cern/root/bin/thisroot.sh && \
                HDF5_ROOT=$HDF5_ROOT \
                CMAKE_PREFIX_PATH=$HDF5_ROOT \
                /opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake ../code/source"
        }

        stage("Build project") {
            sh "make VERBOSE=1"
        }

    }
}
