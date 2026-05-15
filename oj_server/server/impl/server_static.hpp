#pragma once

    svr.set_base_dir("./resources/wwwroot");
    svr.set_mount_point("/css", "./resources/css");
    svr.set_mount_point("/uploads", "./uploads");
    std::cout << "[INFO] Server binding to 0.0.0.0:8094..." << std::endl;
    svr.listen("0.0.0.0", 8094);
