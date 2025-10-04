/**
 *
 * @file interrupts.cpp
 * @author Celina Yang
 * @author Dennis Chen
 *
 */

#include <interrupts.hpp>

int main(int argc, char **argv)
{

    // vectors is a C++ std::vector of strings that contain the address of the ISR
    // delays  is a C++ std::vector of ints that contain the delays of each device
    // the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;     //!< string to store single line of trace file
    std::string execution; //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    int now_ms = 0; // simulation clock

    const int KERNEL_MODE = 1;    // switch to/from kernel mode with 1ms duration
    const int CTX_RSTR = 10;      // save/restore context with 10ms duration
    const int ISR_START_ADRS = 1; // calculate memory of the ISR start address with 1ms duration
    const int GET_ISR_ADRS = 1;   // get ISR address from vector table with 1ms duration
    const int EXEC_ISR = 40;      // execute ISR body with 40ms duration
    const int EXEC_IRET = 1;      // execute IRET with 1ms duration

    const int DEVICE_COUNT = 32; // number of devices
    std::vector<int> device_finish(DEVICE_COUNT, -1);


    /******************************************************************/

    // parse each line of the input trace file
    while (std::getline(input_file, trace))
    {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (activity == "CPU")
        {
            execution += std::to_string(now_ms) + ", " + std::to_string(duration_intr) + ", CPU burst\n";
            now_ms += duration_intr;
        }
        else if (activity == "SYSCALL")
        {

            // switching to kernel mode
            execution += std::to_string(now_ms) + ", " + std::to_string(KERNEL_MODE) + ", switch to kernel mode\n";
            now_ms += KERNEL_MODE;

            // save context
            execution += std::to_string(now_ms) + ", " + std::to_string(CTX_RSTR) + ", context saved\n";
            now_ms += CTX_RSTR;

            // calculate ISR start address
            int mem_pos = ADDR_BASE + duration_intr * VECTOR_SIZE;

            // convert memory position mem_pos to hexadecimal string
            std::stringstream ss;
            ss << "0x" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << mem_pos;


            execution += std::to_string(now_ms) + ", " + std::to_string(ISR_START_ADRS) + ", find vector " + std::to_string(duration_intr) + " in memory position " + ss.str() + "\n";
            now_ms += ISR_START_ADRS;

            // get ISR address from vector table
            execution += std::to_string(now_ms) + ", " + std::to_string(GET_ISR_ADRS) + ", load address " + vectors.at(duration_intr) + " into the PC\n";
            now_ms += GET_ISR_ADRS;

            // execute ISR body
            // 1. run the ISR
            execution += std::to_string(now_ms) + ", " + std::to_string(EXEC_ISR) + ", SYSCALL: run the ISR (device driver)\n";
            now_ms += EXEC_ISR;

            // 2. transfer data from device to memory
            execution += std::to_string(now_ms) + ", " + std::to_string(EXEC_ISR) + ", transfer data from device to memory\n";
            now_ms += EXEC_ISR;

            // 3. check for errors
            execution += std::to_string(now_ms) + ", " + std::to_string(delays.at(duration_intr)) + ", check for errors\n";
            now_ms += delays.at(duration_intr);

            // execute IRET
            execution += std::to_string(now_ms) + ", " + std::to_string(EXEC_IRET) + ", executing IRET\n";
            now_ms += EXEC_IRET;
        }
        else if (activity == "END_IO")
        {

            if (device_finish[duration_intr] >= 0 && now_ms < device_finish[duration_intr])
            {
                now_ms = device_finish[duration_intr];
            }

            device_finish[duration_intr] = -1; // reset the device

            // switching to kernel mode
            execution += std::to_string(now_ms) + ", " + std::to_string(KERNEL_MODE) + ", switch to kernel mode\n";
            now_ms += KERNEL_MODE;

            // save context
            execution += std::to_string(now_ms) + ", " + std::to_string(CTX_RSTR) + ", context saved\n";
            now_ms += CTX_RSTR;

            // calculate ISR start address
            int mem_pos = ADDR_BASE + duration_intr * VECTOR_SIZE;
            
            // convert memory position mem_pos to hexadecimal string
            std::stringstream ss;
            ss << "0x" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << mem_pos;

            execution += std::to_string(now_ms) + ", " + std::to_string(ISR_START_ADRS) + ", find vector " + std::to_string(duration_intr) + " in memory position " + ss.str() + "\n";
            now_ms += ISR_START_ADRS;

            // get ISR address from vector table
            execution += std::to_string(now_ms) + ", " + std::to_string(GET_ISR_ADRS) + ", load address " + vectors.at(duration_intr) + " into the PC\n";
            now_ms += GET_ISR_ADRS;

            // execute ISR body
            // 1. run the ISR
            execution += std::to_string(now_ms) + ", " + std::to_string(EXEC_ISR) + ", ENDIO: run the ISR (device driver)\n";
            now_ms += EXEC_ISR;

            // 2. check device status
            execution += std::to_string(now_ms) + ", " + std::to_string(delays.at(duration_intr)) + ", check device status\n";
            now_ms += delays.at(duration_intr);

            // execute IRET
            execution += std::to_string(now_ms) + ", " + std::to_string(EXEC_IRET) + ", executing IRET\n";
            now_ms += EXEC_IRET;
        }
        /************************************************************************/
    }

    input_file.close();

    write_output(execution);

    return 0;
}
