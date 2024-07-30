pipeline {
  agent any
  stages {
    stage('Code check') {
      steps {
        git(url: 'https://github.com/EbrahimFadly/FTP-Server-repo', branch: 'main')
      }
    }

    stage('list dir') {
      steps {
        sh 'ls -la'
      }
    }

    stage('build') {
      steps {
        sh 'docker build -f Dockerfile .'
      }
    }

  }
}