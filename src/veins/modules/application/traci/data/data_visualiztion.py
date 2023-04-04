import pandas as pd
import matplotlib.pyplot as plt
import sys

# 读取CSV文件并将数据存储到DataFrame中
file_name = sys.argv[1]
data = pd.read_csv(file_name)

# # 绘制折线图
fig, ax = plt.subplots()
for col in ["density_own", "flow_own", "speed_own", "rcv_speed_avg"]:
    ax.plot(data['time'], data[col], label=col)
ax.legend()
ax.set_xlabel('Time')
ax.set_ylabel('Value')

plt.show()
