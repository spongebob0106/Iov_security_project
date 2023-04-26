import pandas as pd
import matplotlib.pyplot as plt
import sys

# 读取CSV文件并将数据存储到DataFrame中
file_name = sys.argv[1]
data = pd.read_csv(file_name)

# # # 绘制折线图
# fig, ax = plt.subplots()
# for col in ["density_own", "flow_own", "speed_own", "rcv_speed_avg"]:
#     ax.plot(data['time'], data[col], label=col)
# ax.legend()
# ax.set_xlabel('Time')
# ax.set_ylabel('Value')
plt.subplot(221)
plt.scatter(data['time'], data['flow_own'], c = 'r', s = 1)
plt.title("flow_own")
plt.subplot(222)
plt.scatter(data['time'], data['rcv_speed_avg'], s = 1)
plt.title("rcv_speed_avg")
plt.subplot(223)
plt.scatter(data['time'], data['density_own'], c = 'y', s = 1)
plt.title("density_own")
# plt.subplot(224)
# plt.scatter(data['time'], data['speed_own'], c = 'r', s = 1, alpha=0.5)
# plt.title("speed_own")
#画出散点图
plt.show()


plt.show()
