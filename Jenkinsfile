/*
 * h5cc_test Jenkinsfile
 */

node ("qt") {

    stage("Checkout projects") {
        checkout scm
    }

    dir("build") {
        stage("Run CMake") {
            sh 'rm -rf ./*'
            sh '/opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake --version'
            sh "source /opt/cern/root/bin/thisroot.sh && \
                HDF5_ROOT=/opt/dm_group/hdf5 \
                CMAKE_PREFIX_PATH=/opt/dm_group/hdf5 \
                /opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake ../code/source"
        }

        stage("Build project") {
            sh "make VERBOSE=1"
        }

    }
}
