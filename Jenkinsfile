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
            sh "source /opt/cern/root/bin/thisroot.sh && \
                HDF5_ROOT=/opt/dm_group/hdf5 \
                CMAKE_PREFIX_PATH=/opt/dm_group/hdf5 \
                cmake ../code/source"
        }

        stage("Build project") {
            sh "make VERBOSE=1"
        }

    }
}
