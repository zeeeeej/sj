

./build_samples.sh
rm build/sample_camera_rst.xz
xz -k build/sample_camera_rst
cp build/sample_camera_rst.xz  ~/win_shar/wap
sync
