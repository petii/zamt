#! /bin/sh

for dir in */
do
  echo "Compiling shaders in $dir"
  cd $dir
  glslangValidator -V shader.*
  cd ..
done
