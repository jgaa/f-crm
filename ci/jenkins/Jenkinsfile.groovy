#!/usr/bin/env groovy

pipeline {
    agent { label 'master' }

    environment {
        F_CRM_VERSION = "0.2.0"
    }

    stages {
        stage('Build') {
           parallel {
                stage('Docker-build inside: ubuntu-trusty (AppImage target)') {
                    // Note: We build a container with g++ 7.4 and QT 5.10
                    agent {
                        dockerfile {
                            filename 'Dockefile.ubuntu-trusty'
                            dir 'ci/jenkins'
                            label 'master'
                            
                            // Dangerous, but we need fuse
                            args '--device /dev/fuse --privileged'
                        }
                    }

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        DIST_NAME = 'ubuntu-trusty-'
                        QTDIR = "/opt/qt510"
                    }

                    steps {
                        echo "Building on ubuntu-trusty-AMD64 in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-appimage.sh'
                    }

                    post {
                        success {
                            echo "Build of debian package suceeded!"
                            archive "dist/*.AppImage"
                        }
                    }
                }
                stage('Docker-build inside: ubuntu-xenial (.deb target)') {
                    agent {
                        dockerfile {
                            filename 'Dockefile.ubuntu-xenial'
                            dir 'ci/jenkins'
                            label 'master'
                        }
                    }

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        DIST_NAME = 'ubuntu-xenial-'
                    }

                    steps {
                        echo "Building on ubuntu-xenial-AMD64 in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-deb.sh'
                    }

                    post {
                        success {
                            echo "Build of debian package suceeded!"
                            archive "dist/*.deb"
                        }
                    }
                }
                stage('Docker-build inside: ubuntu-bionic (.deb target)') {
                    agent {
                        dockerfile {
                            filename 'Dockefile.ubuntu-bionic'
                            dir 'ci/jenkins'
                            label 'master'
                        }
                    }

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        DIST_NAME = 'ubuntu-bionic-'
                    }

                    steps {
                        echo "Building on ubuntu-bionic-AMD64 in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-deb.sh'
                    }

                    post {
                        success {
                            echo "Build of debian package suceeded!"
                            archive "dist/*.deb"
                        }
                    }
                }
                stage('Docker-build inside: debian-stretch (.deb target)') {
                    agent {
                        dockerfile {
                            filename 'Dockefile.debian-stretch'
                            dir 'ci/jenkins'
                            label 'master'
                        }
                    }

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        DIST_NAME = 'debian-stretch-'
                    }

                    steps {
                        echo "Building on debian-stretch-AMD64 in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-deb.sh'
                    }

                    post {
                        success {
                            echo "Build of debian package suceeded!"
                            archive "dist/*.deb"
                        }
                    }
                }
                stage('Docker-build inside: debian-testing (.deb target)') {
                    agent {
                        dockerfile {
                            filename 'Dockefile.debian-testing'
                            dir 'ci/jenkins'
                            label 'master'
                        }
                    }

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        DIST_NAME = 'debian-testing-'
                    }

                    steps {
                        echo "Building on debian-testing-AMD64 in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-deb.sh'
                    }

                    post {
                        success {
                            echo "Build of debian package suceeded!"
                            archive "dist/*.deb"
                        }
                    }
                }
                stage('Build on macOS') {
                    agent {label 'macos'}

                    environment {
                        DIST_DIR = "${WORKSPACE}/dist"
                        BUILD_DIR = "${WORKSPACE}/build"
                        SRC_DIR = "${WORKSPACE}"
                        QTDIR="/Users/jgaa/Qt/5.12.0/clang_64"
                    }

                    steps {
                        echo "Building on macOS in ${WORKSPACE}"
                        checkout scm
                        sh 'pwd; ls -la;'
                        sh './scripts/package-macos.sh'
                    }

                    post {
                        success {
                            echo "Build of macOS package succeeded!"
                            archive "dist/*.dmg"
                        }
                    }
                }
                stage('Build on Windows') {
                    agent {label 'windows'}

                    environment {
                        QTDIR="C:\\Qt\\5.10.0\\msvc2017_64"
                    }

                    // The ${WORKSPACE} has the wrong slashes for Windows,
                    // so we will use %cd% to get a path to the workspavce
                    // in a format Windows can work with. From that we will
                    // make the other paths.
                    //
                    steps {
                        echo "Building on Windows in ${WORKSPACE}"
                        checkout scm

                        bat script: '''
                        set SRC_DIR=%cd%
                        set DIST_DIR=%SRC_DIR%\\dist
                        set BUILD_DIR=%SRC_DIR%\\build
                        set MSI_TARGET_DIR=C:\\jenkins\\build\\f-crm-x64
                        call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"
                        cd
                        cd %SRC_DIR%
                        dir
                        call scripts\\package-windows.bat
                        if %errorlevel% neq 0 exit /b %errorlevel%
                        copy "%SRC_DIR%\\res\\icons\\f_crm.ico" "%DIST_DIR%\\f-crm"
                        cd %MSI_TARGET_DIR%
                        del f-crm.msi
                        C:\\devel\\mkmsi\\mkmsi.py --auto-create qt --icon "f_crm.ico" --source-dir "%DIST_DIR%\\f-crm" --wix-root "C:\\Program Files (x86)\\WiX Toolset v3.11" --license C:\\devel\\mkmsi\\licenses\\GPL3.rtf --merge-module "C:\\Program Files (x86)\\Common Files\\Merge Modules\\Microsoft_VC140_CRT_x64.msm" --add-desktop-shortcut --project-version %F_CRM_VERSION% --description "CRM for Freelancers and Independent Contractors" --manufacturer "The Last Viking LTD" f-crm
                        if %errorlevel% neq 0 exit /b %errorlevel%
                        copy f-crm.msi %DIST_DIR%\\f-crm-%F_CRM_VERSION%-x64.msi
                        if %errorlevel% neq 0 exit /b %errorlevel%
                        copy f-crm.json %DIST_DIR%\\f-crm-%F_CRM_VERSION%-x64.json
                        if %errorlevel% neq 0 exit /b %errorlevel%
                        echo "Everything is OK"
                        '''
                    }

                    post {
                        success {
                            echo "Build of Windows package succeeded!"
                            archive "dist/*.msi"
                            archive "dist/*.json"
                        }
                    }
                }
            }
        }
    }
}

