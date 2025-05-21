1. 문제점 boostio가 없었음
이걸 어떻게 처리할것인가?

-> 공식사이트에서 제공하는거 다운
-> 사내 보안문제인지 b2.exe 파일이 생성되지 않음

-> 라이브러리 파일을 바로 받자

-> https://sourceforge.net/projects/boost/files/boost-binaries/1.88.0/ 에서 받아서 msvc ~.exe 파일 설치 후 경로 지정.
-> vscode 에서 c_cpp_properties.json 에 include path 지정( 기본이 c:\local\{boost} )
-> task.json 에도 args에 LIBPATH 추가해주자. "/LIBPATH:C:/local boost_1_88_0"