  * ```config.c``` currently uses GIO to open the configuration file
    (```g_file_new_for_commandline_arg()``` followed by
    ```g_file_load_contents()``` and ```g_key_file_load_from_data()```. This was
    originally intended, so things like ```LAIK_TCP_CONFIG=http://foo/bar```
    would "just work", but GIO internally uses GVFS here, so this will only work
    if the system has GVFS installed and running (e.g. run ```apt-get install
    gvfs-backends``` and reboot). Since this is unlikely on HPC systems, we
    might consider dropping this and using the plain
    ```g_key_file_load_from_file()``` directly. Alternatively we could use a
    proper HTTP library to implement HTTP support.
    [libsoup](https://developer.gnome.org/libsoup/stable) seems like a good fit,
    and it should be as simple as ```soup_session_new()```,
    ```soup_session_request()```, ```soup_request_send()```, and finally
    ```g_input_stream_read_all()```. Alternatively
    [libcurl](https://curl.haxx.se/libcurl/) would probably also work, but seems
    a bit harder to integrate with our current GLib-themed code.

  * ```config.c``` currently stores the task addresses directly as strings
    inside the ```addresses``` member of the ```Laik_Tcp_Config``` struct. These
    only get translated to ```struct sockaddr``` objects when creating a new
    socket (in ```socket.c```'s ```laik_tcp_socket_new()``` function). This
    means we are potentially doing an expensive DNS lookup everytime we attempt
    to establish a new connection, which is unnecessary and a potential
    performance problem. While the issue is alleviated by the fact that we are
    aggressively re-using connections and can be further minimized by using IP
    addresses instead of DNS names in the configuration file and/or using a
    caching local resolver such as ```systemd-resolved```, this is probably
    something which should be fixed in the long run. The idea would be that the
    ```addresses``` member stores fully parsed ```struct sockaddr``` objects
    instead of strings, but this requires great care since the default
    configuration object (generated by ```laik_tcp_config_new_default()```) must
    not depend on any code path which can potentially throw errors, and running
    ```getaddrinfo``` can definitely do that.

  * We currently update the example configuration file (```config.txt```)
    manually when we change ```config.{c,h}```, which is error-prone. Ideally,
    we would have a single, machine-readable specification of what configuration
    options are available and what their meaning is and generate the example
    configuration file from that. For example, we could add some magic
    ```BEGIN``` and ```END``` comments to ```config.c``` and have a small script
    parse ```config.c``` for anything between these magic comments.

  * We have this nice statistics module (```stats.c```), but currently only use
    it for debugging performance problems by adding ```laik_tcp_stats_*()```
    calls in areas currently of interest and remove them again after the problem
    has been identified and fixed. Since all the ```laik_tcp_stats_*()``` calls
    become NOPs without ```-D LAIK_TCP_STATS``` anyway, we might consider adding
    a few such calls permanently (i.e. commit them into git). However, we have
    to be careful here since these calls may be quite expensive (each call
    usually does a ```printf()``` call and hash table lookup, so adding them in
    hot code paths might actually create fake performance problems, making the
    debugging of real problems with the technique described above more
    difficult.